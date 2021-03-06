/**
 * CEL(C Extension Library)
 * Copyright (C)2008 - 2018 Hu Jinya(hu_jinya@163.com)
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 */
#ifndef __CEL_NET_IP_WIN_H__
#define __CEL_NET_IP_WIN_H__

#include "cel/types.h"

/*
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |Version|Hdr Len|Type of service|          Total Length         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Identifiler                 |Flags  |  Fragmented Offset    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Time to Live  |   Protocol    |      Header Checksum          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                         Source Address                        |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                        Destination Address                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |           Options                     |      Padding          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
typedef struct iphdr
{
    U8 ihl:4;              /**< Length of the header unsigned int */
    U8 version:4; 
    U8 tos;
    U16 tot_len;
    U16 id;
    U16 frag_off;
    U8 ttl;
    U8 protocol;
    U16 check;
    U32 saddr;
    U32 daddr;
}CelIpHdr;

typedef struct icmp  
{  
    U8 icmp_type;  /* type of message, see below */  
    U8 icmp_code;  /* type sub code */  
    U16 icmp_cksum; /* ones complement checksum of struct */  
    union  
    {  
        u_char ih_pptr;     /* ICMP_PARAMPROB */  
        struct in_addr ih_gwaddr;   /* gateway address */  
        struct ih_idseq     /* echo datagram */  
        {  
            U16 icd_id;  
            U16 icd_seq;  
        }ih_idseq;  
        U32 ih_void;  
        /* ICMP_UNREACH_NEEDFRAG -- Path MTU Discovery (RFC1191) */  
        struct ih_pmtu  
        {  
            U16 ipm_void;  
            U16 ipm_nextmtu;  
        }ih_pmtu;  
        struct ih_rtradv  
        {  
            U8 irt_num_addrs;  
            U8 irt_wpa;  
            U16 irt_lifetime;  
        }ih_rtradv;  
    }icmp_hun;  
#define icmp_pptr       icmp_hun.ih_pptr  
#define icmp_gwaddr     icmp_hun.ih_gwaddr  
#define icmp_id         icmp_hun.ih_idseq.icd_id  
#define icmp_seq        icmp_hun.ih_idseq.icd_seq  
#define icmp_void       icmp_hun.ih_void  
#define icmp_pmvoid     icmp_hun.ih_pmtu.ipm_void  
#define icmp_nextmtu    icmp_hun.ih_pmtu.ipm_nextmtu  
#define icmp_num_addrs  icmp_hun.ih_rtradv.irt_num_addrs  
#define icmp_wpa        icmp_hun.ih_rtradv.irt_wpa  
#define icmp_lifetime   icmp_hun.ih_rtradv.irt_lifetime  
    union  
    {  
        struct  
        {  
            U32 its_otime;  
            U32 its_rtime;  
            U32 its_ttime;  
        }id_ts;  
        struct  
        {  
            struct iphdr idi_ip;  
            /* options and then 64 bits of data */  
        }id_ip;  
        //struct icmp_ra_addr id_radv;  
        U32 id_mask;  
        U8 id_data[1];  
    } icmp_dun;  
#define icmp_otime  icmp_dun.id_ts.its_otime  
#define icmp_rtime  icmp_dun.id_ts.its_rtime  
#define icmp_ttime  icmp_dun.id_ts.its_ttime  
#define icmp_ip     icmp_dun.id_ip.idi_ip  
#define icmp_radv   icmp_dun.id_radv  
#define icmp_mask   icmp_dun.id_mask  
#define icmp_data   icmp_dun.id_data  
}CelIcmp;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif
