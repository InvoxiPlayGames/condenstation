#include <cell/cell_fs.h>
#include <sys/random_number.h>
#include <netex/libnetctl.h>
#include <sysutil/sysutil_userinfo.h>
#include <sys/ss_get_open_psid.h>
#include <cell/hash/libsha1.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ppcasm.h>

#include "aes.h"
#include "condenstation_logger.h"

// loaded config globals
bool HasConfigLoaded = false;
char SteamAccountName[128];
char SteamAccessToken[1024];
char SteamGuardData[1024];

#define CST_CONFIG_DEC_HDR 0x43445354
#define CST_CONFIG_ENC_HDR 0x43535445

typedef struct _cst_config_v1 {
    uint32_t magic;
    int version;
    uint8_t align[8];
    char accountName[128];
    char accessToken[1024];
    char guardData[1024];
} cst_config_v1_t;

typedef struct _cst_encrypted_config_v1 {
    uint32_t magic;
    int version;
    uint32_t cih;
    uint32_t crc;
    uint8_t iv[0x10];
    cst_config_v1_t encrypted;
} cst_encrypted_config_v1_t;

CellUserInfoUserStat currentUserStat;

// defined in crc32.c
void crc32(const void *data, size_t n_bytes, uint32_t *crc);

static bool v1_encryption_available()
{
    // v1 encryption relies on getting the MAC address
    int cstate;
    cellNetCtlGetState(&cstate);
    return (cstate > CELL_NET_CTL_STATE_Connecting);
}

static uint32_t cinfohash = 0x69696969;
inline void generate_encryption_key_v1(uint8_t *out_key)
{
    // create a buffer and initialize it
    uint8_t keybuf[0x20];
    memset(keybuf, 0x22, sizeof(keybuf));

    // get user info
    CellUserInfoUserStat st;
    cellUserInfoGetStat(CELL_SYSUTIL_USERID_CURRENT, &st);
    memcpy(keybuf + 0x0, &st.id, sizeof(st.id));

    // get console info
    union CellNetCtlInfo info;
    cellNetCtlGetInfo(CELL_NET_CTL_INFO_ETHER_ADDR, &info);
    memcpy(keybuf + 0xC, info.ether_addr.data, 6);
    sys_ss_get_open_psid((CellSsOpenPSID *)(keybuf + 0x4)); // sorry

    // the key is the last 0x10 bytes of the sha1 hash 
    uint8_t out_hash[0x14];
    cellSha1Digest(keybuf, sizeof(keybuf), out_hash);
    memset(keybuf, 0x41, sizeof(keybuf)); // clear the key buf since we don't need it anymore
    memcpy(out_key, out_hash + 0x4, 0x10);

    // create a crc32 of the key hash and then clear it
    cinfohash = 0;
    crc32(out_hash, sizeof(out_hash), &cinfohash);
    memset(out_hash, 0x41, sizeof(out_hash));
}

static void encrypt_config_v1(cst_config_v1_t *input, cst_encrypted_config_v1_t *output)
{
    uint8_t aes_key[0x10];
    uint8_t aes_iv[0x10];
    struct AES_ctx ctx;
    uint32_t data_crc = 0;
    // generate a CRC checksum of the input data
    crc32(input, sizeof(cst_config_v1_t), &data_crc);
    // generate a random IV
    sys_get_random_number(aes_iv, sizeof(aes_iv));
    // build an encryption key using some console-specific data
    generate_encryption_key_v1(aes_key);
    AES_init_ctx_iv(&ctx, aes_key, aes_iv);
    memset(aes_key, 0x41, sizeof(aes_key)); // clear these from RAM since they aren't needed anymore

    // initialise the output data struct
    output->magic = CST_CONFIG_ENC_HDR;
    output->version = 1;
    output->cih = cinfohash;
    output->crc = data_crc;
    memcpy(output->iv, aes_iv, sizeof(output->iv));

    // copy the input buffer to the output buffer
    memcpy(&output->encrypted, input, sizeof(cst_config_v1_t));

    // encrypt it
    memset(aes_iv, 0x41, sizeof(aes_iv));
    AES_CBC_encrypt_buffer(&ctx, (uint8_t *)&output->encrypted, sizeof(cst_config_v1_t));
    memset(&ctx, 0x41, sizeof(ctx)); // clear the context from RAM
}

static bool decrypt_config_v1(cst_encrypted_config_v1_t *input, cst_config_v1_t *output)
{
    // get the decryption key to decrypt the content
    uint8_t aes_key[0x10];
    struct AES_ctx ctx;
    generate_encryption_key_v1(aes_key);
    AES_init_ctx_iv(&ctx, aes_key, input->iv);
    memset(aes_key, 0x41, sizeof(aes_key)); // clear it from RAM, it isn't needed

    // check if the info hash in the encrypted config matches
    if (input->cih != cinfohash) {
        cdst_log("console info hash in config (%08x) doesn't match current console (%08x)\n", input->cih, cinfohash);
        memset(&ctx, 0x41, sizeof(ctx)); // clear the context from RAM
        return false;
    }
    // decrypt the content
    memcpy(output, &input->encrypted, sizeof(cst_config_v1_t));
    AES_CBC_decrypt_buffer(&ctx, (uint8_t *)output, sizeof(cst_config_v1_t));
    memset(&ctx, 0x41, sizeof(ctx)); // clear the context from RAM

    // verify the decryption was correct
    if (output->magic != CST_CONFIG_DEC_HDR || output->version != 1) {
        cdst_log("config didn't decrypt correctly\n");
        return false;
    }
    uint32_t crc = 0;
    crc32(output, sizeof(cst_config_v1_t), &crc);
    if (input->crc != crc) {
        cdst_log("crc32 in config (%08x) doesn't match (%08x)\n", input->crc, crc);
        return false;
    }

    return true;
}

bool load_auth_config() {
    HasConfigLoaded = false;
    memset(SteamAccountName, 0, sizeof(SteamAccountName));
    memset(SteamAccessToken, 0, sizeof(SteamAccessToken));
    memset(SteamGuardData, 0, sizeof(SteamGuardData));

    // build the filename
    char config_filename[128];
    CellUserInfoUserStat userst;
    cellUserInfoGetStat(CELL_SYSUTIL_USERID_CURRENT, &userst);
    snprintf(config_filename, sizeof(config_filename), "/dev_hdd0/tmp/condenstation_auth_%08x.bin", userst.id);
    cdst_log("checking for config at '%s'...\n", config_filename);

    // open and read the file
    int fd = -1;
    CellFsErrno r = cellFsOpen(config_filename, CELL_FS_O_RDONLY, &fd, NULL, 0);
    if (r != CELL_FS_SUCCEEDED)
        return false; // config not found
    uint8_t configBuffer[4096];
    memset(configBuffer, 0, sizeof(configBuffer));
    uint64_t bytesRead = 0;
    cellFsRead(fd, configBuffer, sizeof(configBuffer), &bytesRead);
    cellFsClose(fd);

    cst_config_v1_t dec_config;
    // v1 decrypted config
    if (*(uint32_t *)configBuffer == CST_CONFIG_DEC_HDR && *(uint32_t *)(configBuffer + 4) == 1) {
        memcpy(&dec_config, configBuffer, sizeof(cst_config_v1_t));
    // v1 encrypted config
    } else if (*(uint32_t *)configBuffer == CST_CONFIG_ENC_HDR && *(uint32_t *)(configBuffer + 4) == 1) {
        if (!v1_encryption_available()) {
            cdst_log("file is encrypted, but we don't have encryption available\n");
            return false;
        }
        if (!decrypt_config_v1((cst_encrypted_config_v1_t *)configBuffer, &dec_config)) {
            cdst_log("config decryption failed\n");
            return false;
        }
    } else {
        cdst_log("unknown config file\n");
        return false;
    }

    // just to be safe - make sure they don't exceed their proper lengths
    dec_config.accountName[sizeof(dec_config.accountName) - 1] = 0;
    dec_config.accessToken[sizeof(dec_config.accessToken) - 1] = 0;
    dec_config.guardData[sizeof(dec_config.guardData) - 1] = 0;
    // copy them into globals
    strncpy(SteamAccountName, dec_config.accountName, sizeof(SteamAccountName));
    strncpy(SteamAccessToken, dec_config.accessToken, sizeof(SteamAccessToken));
    strncpy(SteamGuardData, dec_config.guardData, sizeof(SteamGuardData));
    HasConfigLoaded = true;
}

void save_auth_config() {
    // build the filename
    char config_filename[128];
    CellUserInfoUserStat userst;
    cellUserInfoGetStat(CELL_SYSUTIL_USERID_CURRENT, &userst);
    snprintf(config_filename, sizeof(config_filename), "/dev_hdd0/tmp/condenstation_auth_%08x.bin", userst.id);
    cdst_log("saving config to '%s'...\n", config_filename);

    // prepare the decrypted config buffer
    cst_config_v1_t dec_config;
    dec_config.magic = CST_CONFIG_DEC_HDR;
    dec_config.version = 1;
    strncpy(dec_config.accountName, SteamAccountName, sizeof(dec_config.accountName));
    strncpy(dec_config.accessToken, SteamAccessToken, sizeof(dec_config.accessToken));
    strncpy(dec_config.guardData, SteamGuardData, sizeof(dec_config.guardData));

    // open the file for writing
    int fd;
    CellFsErrno r = cellFsOpen(config_filename, CELL_FS_O_CREAT | CELL_FS_O_TRUNC | CELL_FS_O_WRONLY, &fd, NULL, 0);
    if (r != CELL_FS_SUCCEEDED) {
        cdst_log("file creation failed\n", config_filename);
        return;
    }
    uint64_t bytesWrite = 0;

    // check if we can encrypt the file
    if (v1_encryption_available()) {
        cst_encrypted_config_v1_t enc_config;
        encrypt_config_v1(&dec_config, &enc_config);
        cdst_log("file is being saved encrypted\n", config_filename);
        cellFsWrite(fd, &enc_config, sizeof(enc_config), &bytesWrite);
    } else {
        cdst_log("!! FILE IS BEING SAVED DECRYPTED !!\n");
        cellFsWrite(fd, &dec_config, sizeof(dec_config), &bytesWrite);
    }

    // close the file
    cellFsClose(fd);
}
