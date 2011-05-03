%.o: %.s %.S 
	@echo AS  $<
	$(AS) -o $@ $<

%.o: %.c
	@echo CC  $<
	$(CC) $(CFLAGS) -c -o $@ $<

ifeq ($(TOPDIR)/.depend,$(wildcard $(TOPDIR)/.depend))
include $(TOPDIR)/.depend
endif
