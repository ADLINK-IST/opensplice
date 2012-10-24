/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/* packet-ospl.c
 * Routines for ospl dissection
 * $Id: packet-ospl.c,v 1.1 2008-12-17 14:32:43 eorb Exp $
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <epan/packet.h>
#include <epan/prefs.h>

/* IF OSPL exposes code to other dissectors, then it must be exported
   in a header file. If not, a header file is not needed at all.
#include "packet-ospl.h" */

/* define default ports: can be several ranges, for ex:
  "32828-32830,32832"  */
#define OSPL_UDP_PORTS "53370,53380,53390"


/* Forward declaration we need below */
void proto_reg_handoff_ospl(void);

/* Initialize the protocol and registered fields */
static int proto_ospl = -1;

/* here are not all possible values */
static const value_string flagnames[] = {
	{ 0x0, "NO_FLAGS" },
	{ 0x1, "CONTROL" },
	{ 0x2, "RELIABLE" },
	{ 0x4, "FRAGMENTED" },
	{ 0x6, "REL.FRAGM." },
	{ 0x8, "TERMINATOR" },
	{ 0xa, "REL.TERM." },
	{ 0xc, "FRAGM.TERM." },
	{ 0xe, "REL.FRAGM.TERM."},
	{ 0x10, "P2P" },
	{ 0, NULL }
};

static gint hf_ospl_flags = -1;
static gint hf_ospl_fl_control = -1;
static gint hf_ospl_fl_reliable = -1;
static gint hf_ospl_fl_fragmented = -1;
static gint hf_ospl_fl_terminator = -1;
static gint hf_ospl_fl_p2p = -1;
static gint hf_ospl_snode = -1;
static gint hf_ospl_plength = -1;
static gint hf_ospl_rnode = -1;
static gint hf_ospl_partition = -1;
static gint hf_ospl_messages = -1;
static gint hf_ospl_packet = -1;
static gint hf_ospl_message = -1;
static gint hf_ospl_fragm_tree = -1;
static gint hf_ospl_fragment = -1;
static gint hf_ospl_tmessage = -1;
static gint hf_ospl_tfragm_tree = -1;
static gint hf_ospl_tfragment = -1;
static gint hf_ospl_msg_tree = -1;
static gint hf_ospl_mlength = -1;
static gint hf_ospl_data = -1;
static gint hf_ospl_chstatus = -1;
static gint hf_ospl_ack_tree = -1;
static gint hf_ospl_apartition = -1;
static gint hf_ospl_astart = -1;
static gint hf_ospl_aend = -1;
static gint hf_ospl_error = -1;

/* Initialize the subtree pointers */
static gint ett_ospl = -1;
static gint ett_ospl_flags = -1;
static gint ett_ospl_fragm = -1;
static gint ett_ospl_tfragm = -1;
static gint ett_ospl_message = -1;
static gint ett_ospl_ack = -1;

static dissector_handle_t ospl_handle;

/* the configured range of ports for ospl  */
static range_t *global_ospl_ports = NULL;

/* the currently used range of ports for ospl  */
static range_t *ospl_ports = NULL;


/* Code to actually dissect the packets */
static int
dissect_ospl(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree)
{
	guint32  flags, plen=0, msgs=0, mlen=0, packet=0, rnode=0 ;
	guint32  offset=0, i, p1, p2, ackpart ;

	guint32 FLAG_CONTROL    = 0x1;
	guint32 FLAG_RELIABLE   = 0x2;
	guint32 FLAG_FRAGMENTED = 0x4;
	guint32 FLAG_TERMINATOR = 0x8;
	guint32 FLAG_P2P = 0x10;

        gchar * spl1 = NULL;

	gboolean split = 0 ; /* handle too short packets */

	/* expected min packet length :
	 * 40(44) for control packet or 52 for data packet */
	guint exp_mplen = 0;

	/* Set up structures needed to add the protocol subtree and manage it */
	proto_item *ti;
	proto_tree *ospl_tree, *subtree ;

	static const int *flag_fields[] = {
		&hf_ospl_fl_control,
		&hf_ospl_fl_reliable,
		&hf_ospl_fl_fragmented,
		&hf_ospl_fl_terminator,
		&hf_ospl_fl_p2p,
		NULL
	};

	/* ==========================================================
	 * check if the packet cannot possibly belong to OSPL protocol
	 * ========================================================== */

	/* ignore packets shorter than 8 bytes */
	if(tvb_length(tvb) < 8)
		return 0;

	/* SPL1 */
	spl1 = tvb_get_ephemeral_string(tvb, 0, 4);

	if( spl1[0] != 'S'
	 || spl1[1] != 'P'
	 || spl1[2] != 'L')
		return 0;

	/* flags TODO: big endian or little endian? */
	/* flags = tvb_get_ntohl(tvb, 4); */
	flags = tvb_get_letohl(tvb, 4);

	/* smallest packet size for OSPL
	 * TODO: increase the size when adding fields (44) */
	if(flags & FLAG_CONTROL)
		exp_mplen = 40;
	else    exp_mplen = 52;

	if (tvb_length(tvb) < exp_mplen )
		split = 1;

	/* ==========================================================
	 *	here we are sure that it is an OSPL packet
	 * ==========================================================
	 */

	/* Make entries in Protocol column and Info column on summary display */
	if (check_col(pinfo->cinfo, COL_PROTOCOL))
		col_set_str(pinfo->cinfo, COL_PROTOCOL, "OSPL");
	if (check_col(pinfo->cinfo, COL_INFO))
		col_clear(pinfo->cinfo, COL_INFO);

	/* ==========================================================
	 *		Info column for a control packet
	 * ==========================================================
	 */
	if(flags & FLAG_CONTROL)
	{
		if(split)
		{
			if(check_col(pinfo->cinfo, COL_INFO))
			  col_add_fstr(pinfo->cinfo, COL_INFO,
				"Control packet, too short (%u bytes) !",
				tvb_length(tvb));
		}
		else
		{
			/* number of messages */
			msgs = tvb_get_ntohl(tvb, 20);

			if(check_col(pinfo->cinfo, COL_INFO))
			  col_add_fstr(pinfo->cinfo, COL_INFO,
			  "Control packet, %u message(s)", msgs );
		}
	}
	/* ==========================================================
	 *		Info column for a data packet
	 * ==========================================================
	 */
	else
	{
		if(split)
		{
			if (check_col(pinfo->cinfo, COL_INFO))
			  col_add_fstr(pinfo->cinfo, COL_INFO,
				"Data packet, too short (%u bytes) !",
				tvb_length(tvb));
		}
		else
		{
		  /* packet length */
		  plen = tvb_get_ntohl(tvb, 12);

		  /* number of messages */
		  msgs = tvb_get_ntohl(tvb, 24);
		  /* packet number */
		  packet = tvb_get_ntohl(tvb, 28);

		  if (check_col(pinfo->cinfo, COL_INFO))
		  {
			col_add_fstr(pinfo->cinfo, COL_INFO,
			  "Data packet, flags: %s, packet length: %u, %u message(s)",
			   val_to_str(flags, flagnames,"%x"),plen, msgs );

			  /* display packet number only if reliable flag is set */
			  if( flags & FLAG_RELIABLE)
				col_append_fstr(pinfo->cinfo, COL_INFO,
				  ", packet number %u", packet);
		  }
		}
	} /* packet with data messages */


	/* ==========================================================
	 * 			Tree
	 * ==========================================================
	 */
	if (tree) {

		/* create display subtree for the protocol */
		ti = proto_tree_add_item(tree, proto_ospl, tvb, 0, -1, FALSE);

		ospl_tree = proto_item_add_subtree(ti, ett_ospl);

		/* "SPL1" */
		proto_item_append_text(ti, ", Version %c", spl1[3] );

		/* flags as a bitmask */
		proto_tree_add_bitmask(ospl_tree, tvb, 4, hf_ospl_flags,
			ett_ospl_flags, flag_fields, /*TODO big endian? */TRUE);

		if(split)
		{
			proto_tree_add_item(ospl_tree, hf_ospl_error,tvb, 0, -1, FALSE);
			proto_item_append_text(ti,
				  ": Packet is too short (found %u bytes, expected min. %u bytes)",
				   tvb_length(tvb), exp_mplen );

			return tvb_length(tvb);
		}

		/* sending node Id */
		proto_tree_add_item(ospl_tree, hf_ospl_snode, tvb, 8, 4, FALSE);

		/* ==========================================================
		 *		 Tree for a control packet
		 * ==========================================================
		 */
		if(flags & FLAG_CONTROL) /* control packet (ack message) */
		{
		  /* TODO: the packet length field should be
		   * in the message but currently it is not
		   * add field for packet length and shift offset in other fields
 		   * plen = tvb_get_ntohl(tvb, 12 );
		   * proto_tree_add_item(ospl_tree, hf_ospl_plength, tvb, 12, 4, FALSE); */
		  plen = tvb_length(tvb);

		  /* receiving node id - only applicable for P2P */
		  rnode = tvb_get_ntohl(tvb, 12);

		  if( flags & FLAG_P2P )
			proto_tree_add_item (ospl_tree, hf_ospl_rnode, tvb, 12, 4,FALSE);
		  else
			proto_tree_add_uint_format(ospl_tree, hf_ospl_rnode,
				tvb, 12, 4, rnode,
		 	"Receiving Node Id (not applicable, because P2P flag is not set): %u",
				rnode);

		  /* network partition id, number of messages */
		  proto_tree_add_item(ospl_tree, hf_ospl_partition, tvb, 16, 4, FALSE);
		  proto_tree_add_item(ospl_tree, hf_ospl_messages,  tvb, 20, 4, FALSE);

 		  /* check that number of ack messages is correct:
		   * packet_length = header_length + 12* number_of_ack_messages */
		  if( plen != 28 +msgs*12 )
		  {
			ti = proto_tree_add_item(ospl_tree, hf_ospl_error, tvb,
				20, 4 , FALSE);
			proto_item_append_text(ti,
				": Invalid number of ack messages (%u) or packet length (%u): (packet_length = 28 + number_of_ack_messages*12) => expected number of ack messages is %u ",
				msgs, plen, (plen-28)/12 );

			/* display only as much as can be found in the packet */
			if( plen < 28 +msgs*12)
				msgs = (plen-28)/12 ;
		  }

		  /* channel status */
		  proto_tree_add_item(ospl_tree, hf_ospl_chstatus,  tvb, 24, 4, FALSE);

		  offset = 28;

		  /* for each ack message */
		  while (msgs > 0)
			{
			  ti = proto_tree_add_item(ospl_tree, hf_ospl_ack_tree, tvb,
				offset, 12 , FALSE);
			  subtree = proto_item_add_subtree(ti, ett_ospl_ack);

			  proto_tree_add_item(subtree, hf_ospl_apartition, tvb,
				offset, 4, FALSE);
			  proto_tree_add_item(subtree, hf_ospl_astart, tvb,
				offset+4, 4, FALSE);
			  proto_tree_add_item(subtree, hf_ospl_aend, tvb,
				offset+8, 4, FALSE);

			  ackpart = tvb_get_ntohl(tvb, offset);
			  p1 = tvb_get_ntohl(tvb, offset+4);
			  p2 = tvb_get_ntohl(tvb, offset+8);

			  if(p1 != p2)
				proto_item_append_text(ti,
				", Network Partition Id: %u, Acked Packets: %u-%u",
				ackpart, p1, p2);
			  else
				proto_item_append_text(ti,
				", Network Partition Id: %u, Acked Packet: %u",
				ackpart, p1);

			  msgs = msgs -1 ;
			  offset = offset + 12;
		  } /* while */
		}
		/* ==========================================================
		 * 		Tree for a data packet
		 * ==========================================================
		 */
		else
		{
		  /* packet length */
		  proto_tree_add_item(ospl_tree, hf_ospl_plength, tvb, 12, 4, FALSE);

		  /* check that the packet length is correct */
		  if( plen != tvb_length(tvb))
		  {
			ti = proto_tree_add_item(ospl_tree, hf_ospl_error, tvb,
				12, 4 , FALSE);
			proto_item_append_text(ti,
				": Invalid packet length (%u): expected %u ",
				plen, tvb_length(tvb));

			plen = tvb_length(tvb);
		  }

		  /* receiving node id - only applicable for P2P */
		  rnode = tvb_get_ntohl(tvb, 16);
		  if( flags & FLAG_P2P )
			proto_tree_add_item (ospl_tree, hf_ospl_rnode, tvb, 16, 4,FALSE);
		  else
			proto_tree_add_uint_format(ospl_tree, hf_ospl_rnode,
				tvb, 16, 4, rnode,
		 	"Receiving Node Id (not applicable, because P2P flag is not set): %u",
				rnode);

		  /* network partition id, number of messages */
		  proto_tree_add_item(ospl_tree, hf_ospl_partition, tvb, 20, 4, FALSE);
		  proto_tree_add_item(ospl_tree, hf_ospl_messages,  tvb, 24, 4, FALSE);

		  /* packet number - only applicable for reliable transport */
		  if( flags & FLAG_RELIABLE )
			proto_tree_add_item(ospl_tree, hf_ospl_packet, tvb, 28, 4, FALSE);
		  else
			proto_tree_add_uint_format(ospl_tree, hf_ospl_packet,
			  tvb, 28, 4, packet,
			  "Packet Number (not applicable because RELIABLE flag is not set): %u",
			  packet);

		  /* fragmented: only applicable for FRAGMENTED */
		  ti = proto_tree_add_item(ospl_tree, hf_ospl_fragm_tree, tvb,
			32, 8 , FALSE);
		  subtree = proto_item_add_subtree(ti, ett_ospl_fragm);

		  if( !(flags & FLAG_FRAGMENTED))
			proto_item_append_text(ti,
			" (not applicable because FRAGMENTED flag is not set)");

		  proto_tree_add_item(subtree, hf_ospl_message, tvb, 32, 4, FALSE);
		  proto_tree_add_item(subtree, hf_ospl_fragment, tvb, 36, 4, FALSE);

		  if(flags & FLAG_FRAGMENTED)
		  {
			proto_item_append_text(ti,
			", Message: %u, Fragment: %u",
			tvb_get_ntohl(tvb,32), tvb_get_ntohl(tvb, 36));
		  }

		  /* last fragment : only applicable for TERMINATOR */
		  ti = proto_tree_add_item(ospl_tree, hf_ospl_tfragm_tree, tvb,
			40, 8 , FALSE);
		  subtree = proto_item_add_subtree(ti, ett_ospl_tfragm);

		  if( !(flags & FLAG_TERMINATOR))
			proto_item_append_text(ti,
			  " (not applicable because TERMINATOR flag is not set)");

		  proto_tree_add_item(subtree, hf_ospl_tmessage,  tvb, 40, 4, FALSE);
		  proto_tree_add_item(subtree, hf_ospl_tfragment, tvb, 44, 4, FALSE);

		  if(flags & FLAG_TERMINATOR)
		  {
			proto_item_append_text(ti,
			  ", Message: %u, Fragment: %u",
			  tvb_get_ntohl(tvb,40), tvb_get_ntohl(tvb, 44));
		  }

		  offset = 48;     /* where message length is stored */
		  i = 1 ;
		  /* for each message */
		  while (i <= msgs)
		  {
			/* current message length */
			mlen = tvb_get_ntohl(tvb, offset);

			/* check that message length is correct
			 * if there is enough space for the message */
			if( offset +4 +mlen > plen)
			{
				ti = proto_tree_add_item(ospl_tree, hf_ospl_msg_tree, tvb,
					offset, -1 , FALSE);
				subtree = proto_item_add_subtree(ti, ett_ospl_message);

				proto_tree_add_item(subtree, hf_ospl_mlength, tvb,
					offset, 4, FALSE);

				proto_tree_add_item(subtree, hf_ospl_data, tvb,
					offset+4 , -1, FALSE);

				ti = proto_tree_add_item(ospl_tree, hf_ospl_error, tvb,
					offset, -1 , FALSE);
				proto_item_append_text(ti,
				  ": Invalid message length: expected maximal length: %u (bytes %u-%u) found: %u (%u byte(s) more than available)",
				  plen-offset-4, offset+4, plen-1, mlen, offset+4+mlen-plen );

				return offset+4 ;
			}

			ti = proto_tree_add_item(ospl_tree, hf_ospl_msg_tree, tvb,
				offset, mlen+4 , FALSE);
			subtree = proto_item_add_subtree(ti, ett_ospl_message);

			proto_tree_add_item(subtree, hf_ospl_mlength, tvb,
				offset, 4, FALSE);
			proto_item_append_text(ti, ", Length: %u", mlen);

			proto_tree_add_item(subtree, hf_ospl_data, tvb,
				offset+4 , mlen, FALSE);

			/* 4 bytes containing mlen, bytes with data
			 * and 2 bytes padding */
			offset +=  4 + mlen;// + 2 ;
			i++;

			/* check if there is enough space for the next message length field */
			if(i <= msgs && offset+4 > plen)
			{
				ti = proto_tree_add_item(ospl_tree, hf_ospl_error, tvb,
					offset-2, -1 , FALSE);

				proto_item_append_text(ti,
				  ": Invalid number of messages or message length: expected to find the length of message %u in bytes %u-%u, available are only %u byte(s) (%u-%u)",
				   i, offset, offset+3, offset+4-plen, offset-2, plen-1);

				return offset-2 ;
			}
		  } /* while (for each message) */

		   /* check if some bytes left */
		  if( plen > offset)
		  {
			ti = proto_tree_add_item(ospl_tree, hf_ospl_error, tvb,
					offset, -1 , FALSE);

			proto_item_append_text(ti,
			  ": Invalid number of messages or message length: unused %u byte(s): (%u-%u)",
			  plen-offset, offset, plen-1);
		  }

		}/* Tree for a data packet */
	}/* Tree */

	/* If this protocol has a sub-dissector call it here, see section 1.8 */

	/* Return the amount of data this dissector was able to dissect */
	return tvb_length(tvb);
}

static void
ospl_delete_callback(guint32 port)
{
	if ( port ) dissector_delete("udp.port", port, ospl_handle);
}

static void
ospl_add_callback(guint32 port)
{
	if ( port ) dissector_add("udp.port", port, ospl_handle);
}

static void
ospl_reinit(void)
{
	if (ospl_ports)
	{
	  range_foreach(ospl_ports, ospl_delete_callback);
	  g_free(ospl_ports);
	}

	ospl_ports = range_copy(global_ospl_ports);

	range_foreach(ospl_ports, ospl_add_callback);
}


/* Register the protocol with Wireshark */
void
proto_register_ospl(void)
{

	module_t *ospl_module;

	/* Setup list of header fields */
	static hf_register_info hf[] = {

	  { &hf_ospl_flags,{"Flags","ospl.flags",
		FT_UINT32, BASE_HEX, VALS(flagnames), 0x0,
		"Flags describing the content of the packet", HFILL }},

	  { &hf_ospl_fl_control,{"CONTROL","ospl.fl_control",
		FT_UINT32, BASE_DEC,  NULL, 0x1,
		"Control message", HFILL }},

	  { &hf_ospl_fl_reliable,{"RELIABLE","ospl.fl_reliable",
		FT_UINT32, BASE_DEC,  NULL, 0x2,
		"This message must be acked", HFILL }},

	  { &hf_ospl_fl_fragmented,{"FRAGMENTED","ospl.fl_fragmented",
		FT_UINT32, BASE_DEC,  NULL, 0x4,
	    "The packet contains a partial message, that continues in the next packet",HFILL }},

	  { &hf_ospl_fl_terminator,{"TERMINATOR","ospl.fl_terminator",
		FT_UINT32, BASE_DEC,  NULL, 0x8,
	    "This packet contains the end for a partial message from the last packet",HFILL }},

	  { &hf_ospl_fl_p2p,{"P2P","ospl.fl_p2p",
		FT_UINT32, BASE_DEC,  NULL, 0x10,
		"Peer to Peer packet", HFILL }},

	  { &hf_ospl_snode,{ "Sending Node Id", "ospl.snode",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Sending Node Id", HFILL }},

	  { &hf_ospl_plength,{ "Packet Length", "ospl.plength",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Length of complete packet", HFILL }},

	  { &hf_ospl_rnode,{ "Receiving Node Id", "ospl.rnode",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Receiving Node Id, only applicable for P2P", HFILL }},

	  { &hf_ospl_partition,{ "Network Partition Id", "ospl.partition",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Network Partition Id", HFILL }},

	  { &hf_ospl_messages,{ "Number of Messages", "ospl.messages",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Number of messages in this packet", HFILL }},

	  { &hf_ospl_packet,{ "Packet Number", "ospl.packet",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Packet number, only applicable for reliable transport", HFILL }},

	  { &hf_ospl_fragm_tree,  { "Fragment     ", "ospl.fragm_tree",
		FT_NONE,   BASE_NONE, NULL, 0x0,
		"Fragment, only applicable when FRAGMENTED flag is set", HFILL }},

	  { &hf_ospl_message,{ "Fragmented Message Number", "ospl.message",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
	    "Fragmented message number, only applicable when FRAGMENTED flag is set",HFILL }},

	  { &hf_ospl_fragment,{ "Fragment Number", "ospl.fragment",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Fragment number, only applicable when FRAGMENTED flag is set", HFILL }},

	  { &hf_ospl_tfragm_tree, { "Last Fragment", "ospl.tfragm_tree",
		FT_NONE, BASE_NONE,  NULL, 0x0,
		"Last fragment, only applicable when TERMINATOR flag is set", HFILL }},

	  { &hf_ospl_tmessage,{ "Terminated Message Number", "ospl.tmessage",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
	    "Terminated message number, only applicable when TERMINATOR flag is set",HFILL }},

	  { &hf_ospl_tfragment,{ "Terminating Fragment Number", "ospl.tfragment",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
	    "Terminating fragment number, only applicable when TERMINATOR flag is set",HFILL }},

	  { &hf_ospl_msg_tree, { "Message", "ospl.msg_tree",
		FT_NONE,   BASE_NONE, NULL, 0x0,
		"Message", HFILL }},

	  { &hf_ospl_mlength,{ "Message Length", "ospl.mlength",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Length of this message or fragment",HFILL }},

	  { &hf_ospl_data,{ "Data", "ospl.data",
		FT_BYTES,  BASE_NONE, NULL, 0x0,
		"Data", HFILL }},

	  { &hf_ospl_chstatus,{ "Channel Status", "ospl.chstatus",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Number of defragbuffers in use", HFILL }},

	  { &hf_ospl_ack_tree,{ "Ack Message", "ospl.ack_tree",
		FT_NONE,   BASE_NONE, NULL, 0x0,
		"Ack Message", HFILL }},

	  { &hf_ospl_apartition,{ "Network Partition Id", "ospl.apartition",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Network Partition Id of acked messages", HFILL }},

	  { &hf_ospl_astart,{ "Starting Packet Number", "ospl.astart",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Starting packet number of acked range", HFILL }},

	  { &hf_ospl_aend,{ "Closing Packet Number", "ospl.aend",
		FT_UINT32, BASE_DEC,  NULL, 0x0,
		"Closing packet number of acked range", HFILL }},

	  { &hf_ospl_error,{ "### Error", "ospl.error",
		FT_NONE, BASE_NONE,  NULL, 0x0,
		"Dissector error message", HFILL }}
	};

	/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_ospl,
		&ett_ospl_flags,
		&ett_ospl_fragm,
		&ett_ospl_tfragm,
		&ett_ospl_message,
		&ett_ospl_ack
	};

	/* Register the protocol name and description */
	proto_ospl = proto_register_protocol("Open Splice Protocol","OSPL", "ospl");

	/* Required function calls to register the header fields and subtrees used */
	proto_register_field_array(proto_ospl, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));

	/* Register preferences module (See Section 2.6 for more on preferences) */
	ospl_module = prefs_register_protocol(proto_ospl, proto_reg_handoff_ospl);

	/* Set default OSPL port(s) */
	range_convert_str(&global_ospl_ports, OSPL_UDP_PORTS, MAX_UDP_PORT);

	prefs_register_range_preference(ospl_module, "udp.ports",
	  "Open Splice UDP Port(s)",
	  "Set the port(s) for Open Splice messages (default: " OSPL_UDP_PORTS ")",
	  &global_ospl_ports, MAX_UDP_PORT);

	register_init_routine(&ospl_reinit);
}

/* If this dissector uses sub-dissector registration add a registration routine.
   This exact format is required because a script is used to find these
   routines and create the code that calls these routines.

   This function is also called by preferences whenever "Apply" is pressed
   (see prefs_register_protocol above) so it should accommodate being called
   more than once.
*/
void
proto_reg_handoff_ospl(void)
{
	static int ospl_prefs_initialized = FALSE;

	if (!ospl_prefs_initialized) {
		ospl_handle = new_create_dissector_handle(dissect_ospl, proto_ospl);

		ospl_prefs_initialized = TRUE;
	}
	ospl_reinit();
}


