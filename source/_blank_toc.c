// we have a big block of toc, to offset ours and paste in the original module's
char filler_toc[0x800] __attribute__((section(".toc")));
