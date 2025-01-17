// we have a big block of toc, to offset ours and paste in the original module's
char filler_toc[0x8000] __attribute__((section(".toc")));
char toc_register[0] __attribute__((section(".toc")));
