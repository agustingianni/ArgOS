#ifndef ATA_H_
#define ATA_H_

#include <types.h>

/* ATA/ATAPI device parameters */
struct ata_params {
/*000*/ uint16_t       config;                 /* configuration info */
#define ATA_PROTO_MASK                  0x8003
#define ATA_PROTO_ATA                   0x0002
#define ATA_PROTO_ATAPI_12              0x8000
#define ATA_PROTO_ATAPI_16              0x8001
#define ATA_ATAPI_TYPE_MASK             0x1f00
#define ATA_ATAPI_TYPE_DIRECT           0x0000  /* disk/floppy */
#define ATA_ATAPI_TYPE_TAPE             0x0100  /* streaming tape */
#define ATA_ATAPI_TYPE_CDROM            0x0500  /* CD-ROM device */
#define ATA_ATAPI_TYPE_OPTICAL          0x0700  /* optical disk */
#define ATA_DRQ_MASK                    0x0060  
#define ATA_DRQ_SLOW                    0x0000  /* cpu 3 ms delay */
#define ATA_DRQ_INTR                    0x0020  /* interrupt 10 ms delay */
#define ATA_DRQ_FAST                    0x0040  /* accel 50 us delay */

/*001*/ uint16_t       cylinders;              /* # of cylinders */
        uint16_t       reserved2;
/*003*/ uint16_t       heads;                  /* # heads */
        uint16_t       obsolete4;
        uint16_t       obsolete5;
/*006*/ uint16_t       sectors;                /* # sectors/track */
/*007*/ uint16_t       vendor7[3];
/*010*/ uint8_t        serial[20];             /* serial number */
/*020*/ uint16_t       retired20;
        uint16_t       retired21;
        uint16_t       obsolete22;
/*023*/ uint8_t        revision[8];            /* firmware revision */
/*027*/ uint8_t        model[40];              /* model name */
/*047*/ uint16_t       sectors_intr;           /* sectors per interrupt */
/*048*/ uint16_t       usedmovsd;              /* double word read/write? */
/*049*/ uint16_t       capabilities1;
#define ATA_SUPPORT_DMA                 0x0100
#define ATA_SUPPORT_LBA                 0x0200
#define ATA_SUPPORT_OVERLAP             0x4000

/*050*/ uint16_t       capabilities2;
/*051*/ uint16_t       retired_piomode;        /* PIO modes 0-2 */
#define ATA_RETIRED_PIO_MASK            0x0300

/*052*/ uint16_t       retired_dmamode;        /* DMA modes */
#define ATA_RETIRED_DMA_MASK            0x0003

/*053*/ uint16_t       atavalid;               /* fields valid */
#define ATA_FLAG_54_58                  0x0001  /* words 54-58 valid */
#define ATA_FLAG_64_70                  0x0002  /* words 64-70 valid */
#define ATA_FLAG_88                     0x0004  /* word 88 valid */

/*054*/ uint16_t       obsolete54[5];
/*059*/ uint16_t       multi;
#define ATA_MULTI_VALID                 0x0100

/*060*/ uint16_t       lba_size_1;
        uint16_t       lba_size_2;
        uint16_t       obsolete62;
/*063*/ uint16_t       mwdmamodes;             /* multiword DMA modes */ 
/*064*/ uint16_t       apiomodes;              /* advanced PIO modes */ 

/*065*/ uint16_t       mwdmamin;               /* min. M/W DMA time/word ns */
/*066*/ uint16_t       mwdmarec;               /* rec. M/W DMA time ns */
/*067*/ uint16_t       pioblind;               /* min. PIO cycle w/o flow */
/*068*/ uint16_t       pioiordy;               /* min. PIO cycle IORDY flow */
        uint16_t       reserved69;
        uint16_t       reserved70;
/*071*/ uint16_t       rlsovlap;               /* rel time (us) for overlap */
/*072*/ uint16_t       rlsservice;             /* rel time (us) for service */
        uint16_t       reserved73;
        uint16_t       reserved74;
/*075*/ uint16_t       queue;
#define ATA_QUEUE_LEN(x)                ((x) & 0x001f)

        uint16_t       reserved76;
        uint16_t       reserved77;
        uint16_t       reserved78;
        uint16_t       reserved79;
/*080*/ uint16_t       version_major;
/*081*/ uint16_t       version_minor;

        struct {
/*082/085*/ uint16_t   command1;
#define ATA_SUPPORT_SMART               0x0001
#define ATA_SUPPORT_SECURITY            0x0002
#define ATA_SUPPORT_REMOVABLE           0x0004
#define ATA_SUPPORT_POWERMGT            0x0008
#define ATA_SUPPORT_PACKET              0x0010
#define ATA_SUPPORT_WRITECACHE          0x0020
#define ATA_SUPPORT_LOOKAHEAD           0x0040
#define ATA_SUPPORT_RELEASEIRQ          0x0080
#define ATA_SUPPORT_SERVICEIRQ          0x0100
#define ATA_SUPPORT_RESET               0x0200
#define ATA_SUPPORT_PROTECTED           0x0400
#define ATA_SUPPORT_WRITEBUFFER         0x1000
#define ATA_SUPPORT_READBUFFER          0x2000
#define ATA_SUPPORT_NOP                 0x4000

/*083/086*/ uint16_t   command2;
#define ATA_SUPPORT_MICROCODE           0x0001
#define ATA_SUPPORT_QUEUED              0x0002
#define ATA_SUPPORT_CFA                 0x0004
#define ATA_SUPPORT_APM                 0x0008
#define ATA_SUPPORT_NOTIFY              0x0010
#define ATA_SUPPORT_STANDBY             0x0020
#define ATA_SUPPORT_SPINUP              0x0040
#define ATA_SUPPORT_MAXSECURITY         0x0100
#define ATA_SUPPORT_AUTOACOUSTIC        0x0200
#define ATA_SUPPORT_ADDRESS48           0x0400
#define ATA_SUPPORT_OVERLAY             0x0800
#define ATA_SUPPORT_FLUSHCACHE          0x1000
#define ATA_SUPPORT_FLUSHCACHE48        0x2000

/*084/087*/ uint16_t   extension;
        } support, enabled;

/*088*/ uint16_t       udmamodes;              /* UltraDMA modes */
/*089*/ uint16_t       erase_time;
/*090*/ uint16_t       enhanced_erase_time;
/*091*/ uint16_t       apm_value;
/*092*/ uint16_t       master_passwd_revision;
/*093*/ uint16_t       hwres;
#define ATA_CABLE_ID                    0x2000

/*094*/ uint16_t       acoustic;
#define ATA_ACOUSTIC_CURRENT(x)         ((x) & 0x00ff)
#define ATA_ACOUSTIC_VENDOR(x)          (((x) & 0xff00) >> 8)

/*095*/ uint16_t       stream_min_req_size;
/*096*/ uint16_t       stream_transfer_time;
/*097*/ uint16_t       stream_access_latency;
/*098*/ uint32_t       stream_granularity;
/*100*/ uint16_t       lba_size48_1;
        uint16_t       lba_size48_2;
        uint16_t       lba_size48_3;
        uint16_t       lba_size48_4;
        uint16_t       reserved104[23];
/*127*/ uint16_t       removable_status;
/*128*/ uint16_t       security_status;
        uint16_t       reserved129[31];
/*160*/ uint16_t       cfa_powermode1;
        uint16_t       reserved161[14];
/*176*/ uint16_t       media_serial[30];
        uint16_t       reserved206[49];
/*255*/ uint16_t       integrity;
};

/* ATA commands */
#define ATA_NOP                         0x00    /* NOP command */
#define         ATA_NF_FLUSHQUEUE       0x00    /* flush queued cmd's */
#define         ATA_NF_AUTOPOLL         0x01    /* start autopoll function */
#define ATA_ATAPI_RESET                 0x08    /* reset ATAPI device */
#define ATA_READ                        0x20    /* read command */
#define ATA_READ48                      0x24    /* read command */
#define ATA_READ_DMA48                  0x25    /* read w/DMA command */
#define ATA_READ_DMA_QUEUED48           0x26    /* read w/DMA QUEUED command */
#define ATA_READ_MUL48                  0x29    /* read multi command */
#define ATA_WRITE                       0x30    /* write command */
#define ATA_WRITE48                     0x34    /* write command */
#define ATA_WRITE_DMA48                 0x35    /* write w/DMA command */
#define ATA_WRITE_DMA_QUEUED48          0x36    /* write w/DMA QUEUED command */
#define ATA_WRITE_MUL48                 0x39    /* write multi command */
#define ATA_PACKET_CMD                  0xa0    /* packet command */
#define ATA_ATAPI_IDENTIFY              0xa1    /* get ATAPI params*/
#define ATA_SERVICE                     0xa2    /* service command */
#define ATA_READ_MUL                    0xc4    /* read multi command */
#define ATA_WRITE_MUL                   0xc5    /* write multi command */
#define ATA_SET_MULTI                   0xc6    /* set multi size command */
#define ATA_READ_DMA_QUEUED             0xc7    /* read w/DMA QUEUED command */
#define ATA_READ_DMA                    0xc8    /* read w/DMA command */
#define ATA_WRITE_DMA                   0xca    /* write w/DMA command */
#define ATA_WRITE_DMA_QUEUED            0xcc    /* write w/DMA QUEUED command */
#define ATA_SLEEP                       0xe6    /* sleep command */
#define ATA_FLUSHCACHE                  0xe7    /* flush cache to disk */
#define ATA_FLUSHCACHE48                0xea    /* flush cache to disk */
#define ATA_ATA_IDENTIFY                0xec    /* get ATA params */
#define ATA_SETFEATURES                 0xef    /* features command */
#define         ATA_SF_SETXFER          0x03    /* set transfer mode */
#define         ATA_SF_ENAB_WCACHE      0x02    /* enable write cache */
#define         ATA_SF_DIS_WCACHE       0x82    /* disable write cache */
#define         ATA_SF_ENAB_RCACHE      0xaa    /* enable readahead cache */
#define         ATA_SF_DIS_RCACHE       0x55    /* disable readahead cache */
#define         ATA_SF_ENAB_RELIRQ      0x5d    /* enable release interrupt */
#define         ATA_SF_DIS_RELIRQ       0xdd    /* disable release interrupt */
#define         ATA_SF_ENAB_SRVIRQ      0x5e    /* enable service interrupt */
#define         ATA_SF_DIS_SRVIRQ       0xde    /* disable service interrupt */

#endif
