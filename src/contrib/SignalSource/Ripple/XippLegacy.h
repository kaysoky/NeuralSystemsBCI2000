//! \file       XippLegacy.h
//! \brief      EXtensible Instrument Processing Protocol (XIPP) Legacy packet types 
//!             used in early Grapevine development
//! \author     originated by Csaba Christian Gyulai, modified by Shane Guillory
//! \n Contact: support@rppl.com
//! \details    \copydoc group1
//
// (c) Copyright Ripple, LLC. All rights reserved.
//
// This file may be used under the terms of the GNU Lesser General Public
// License version 3.0 as published by the Free Software Foundation
// and appearing in the file LICENSE.LGPL included in the packaging of
// this file.  Please review the following information to ensure GNU
// Lesser General Public Licensing requirements will be met:
// http://www.gnu.org/copyleft/lesser.html
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//

#ifndef XIPP_LEGACY_H
#define XIPP_LEGACY_H


#ifdef _MSC_VER                 // if MS Visual Studio, use public-domain C99 integer type header for MSVC
    #include "stdint_ms.h"      // (available from http://msinttypes.googlecode.com/svn/trunk/stdint.h )
#else                           // otherwise use the standard stdint.h header that everyone else in the world uses
    #include <stdint.h>
#endif

//    #include <fcntl.h>
//    #include <limits.h>
//    #include <stdint.h>
//    #include <stddef.h>
//    #include <sys/types.h>
//    #include <sys/mman.h>
//    #include <unistd.h>
//    #include <string.h>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic XIPP definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! XIPP library versioning info.
static const unsigned int XIPP_LIBRARY_VER_MAJ = 0;         //!< XIPP library major version.
static const unsigned int XIPP_LIBRARY_VER_MIN = 1;         //!< XIPP library minor version.


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enumerations and handle declarations for XIPP entities in cache and other classes 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Enumeration of string lengths (including null termination character).
enum XippStringLengths
{
    XIPP_PROPERTY_STRLEN            = 32,   //!< Length for type and name fields of properties in bytes.
    XIPP_TINY_STRLEN                = 8,    //!< Length of tiny strings in bytes.
    XIPP_SHORT_STRLEN               = 16,   //!< Length of short strings in bytes.
};

//! Handle Type for Entities
typedef int XippEntityHnd;
static const XippEntityHnd XIPP_INVALID_ENTITY = -1;    ///< Invalid Entity Handle

//! Enumeration of Entity types
enum XippEntityType
{  
    XIPP_ENT_INV, //!< Invalid or unallocated entity handle and structure.
    XIPP_ENT_SYS, //!< The one global system entity
    XIPP_ENT_PRC, //!< Processor on the system
    XIPP_ENT_MOD, //!< Module running on a hardware processor
}; 


//! Enumeration of return values of Xipp functions
enum XippReturn
{  
    XIPP_OK,          //!< No error, information passed is current as of last call to change tracking functions
    XIPP_INVALID,     //!< The handle passed item referred to no longer exists (deleted or erroneous)
    XIPP_INTERRUPTED, //!< Target not valid - server in the middle of a modification
    XIPP_OLD,         //!< Information retrieved has been replaced since last call to sync 
    XIPP_CHANGED,     //!< Entity/property queried has changed
};

//! Enumeration of properties of fields of Xipp structures. Bitfield.
enum XippFields
{  
    XIPP_MOD_NO     = 0,        //!< not user modifiable
    XIPP_MOD_YES    = 1,        //!< always user modifiable
    XIPP_MOD_MAYBE  = 2,        //!< could be user modifiable
    XIPP_COMP       = XIPP_MOD_MAYBE,   //!< composite field, i.e., it has subfields
};

//! \brief  Enumeration of meanings of number 0 used in Xipp packets.
//! \details In XIPP packets '0' has multiple meanings depending where and when is it used.
enum XippPacketZeros
{  
    //! Packet is for all NIP, module, or property. 
    //! Non zero value refers to a particular NIP, module, or property.
    XIPP_ALL_0       = 0,
    //! It is a broadcast packet. Non zero value refers to a particular NIP or module.
    XIPP_BROADCAST_0 = 0,
    //! It is a configuration packet. Non zero value means that it is a data packet.
    XIPP_CONFIG_0    = 0,
    //! Packet is from a host. Non zero value refers to a particular NIP or module.
    XIPP_FROM_HOST_0 = 0,
    //! Field is unused. Number can be any number within range, e.g., 0-255, 
    //! but use 0 to allow future extensions of the protocol.
    XIPP_UNUSED_0    = 0,
        
    //! Property belongs to module. Non zero index means that 
    //! property belongs to a group.
    XIPP_MODULE_0    = 0,
    //! Property is not part of the list. Non zero binding
    //! value refers to next property in list.
    XIPP_NO_LIST_0   = 0,
    //! Property is end of the list. Non zero binding
    //! value refers to next property in list.
    XIPP_END_LIST_0   = 0,
    //! Property does not have flags. Non zero values are not defined yet.
    //! Reserved for future use.
    XIPP_NO_FLAGS_0  = 0,
        
    //! Property is disabled. Non zero value means property is enabled.
    XIPP_DISABLED_0  = 0,
    //! Data collection is event driven instead of collecting samples at a 
    //! periodic rate. Non zero value specifies a periodic data collection rate.
    XIPP_NO_RATE_0   = 0,
};

//! \brief  Enumeration of available number of Xipp entities.
//! \sa     XippPktHdr
enum XippNumberOfEntities
{  
    //! Number of processor addresses (2^8). Note that 0 has special meaning,
    //! so only one less processors can be uniquely addressed.
    XIPP_NUM_PRC     = 256,
    //! Number of module addresses per processor (2^8). Note that 0 has special meaning,
    //! so only one less modules can be uniquely addressed.
    XIPP_MOD_PER_PRC = 256,
    //! Number of module addresses in the whole system. See notes above.
    XIPP_NUM_MOD     = XIPP_NUM_PRC * XIPP_MOD_PER_PRC,
    //! Number of data stream addresses per module (2^8). Note that 0 has special meaning,
    //! so only one less data streams can be uniquely addressed.
    XIPP_DS_PER_MOD = 256,
};

//! \brief  Constants for XIPP packets.
//! \sa     XippPktHdr, XippCmdHdr
enum XippPacketConstants
{
    //! Highest possible processor ID.
    XIPP_MAX_PRC_ID             = XIPP_NUM_PRC - 1,
    //! First property ID. 0 has special meaning.
    XIPP_FIRST_PRP_ID           = 1,
};

// Are selected values different or the same?
static const int XIPP_VALUES_DIFF = 0;              //!< Integer values are different
static const int XIPP_VALUES_SAME = 1;              //!< Integer values are the same
static const double XIPP_DBL_VALUES_DIFF = 0.0;     //!< Double values are different
static const double XIPP_DBL_VALUES_SAME = 1.0;     //!< Double values are the same
static const char * XIPP_MULTIPLE_VALUES = "multiple values";   //!< String displayed if multiple different values should be displayed at once.


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Catalog of defined XIPP commands
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! \brief      Operator broadcast message (reserved for future use)
static const uint32_t XIPP_CMD_OPR = 0x0001;

//! \brief      Discovery Query command.
//! \details Sent as XippCmdPkt. Property Id is not defined, set it to 0.
static const uint32_t XIPP_CMD_QRY = 0x0002;

//! \brief      Acknowledgement notification; response to QRY or additions.
//! \details    Sent as XippPrpPkt. It contains property ID of sender and enumerated property.
//! \note       Multiple ACKs may be sent at once without further prompting.
static const uint32_t XIPP_CMD_ACK = 0x0003;

//! \brief      Entity and Property Deleted notification.
//! \details    It is assumed that specified entity, all of its properties and
//!             subentities were deleted.
//!             Sent as XippCmdPkt. Property Id is not defined, set it to 0.
static const uint32_t XIPP_CMD_DEL = 0x0004;

//! \brief      Property Value Change command, i.e., sender requests target to change value of property.
//! \details    Sent as XippPrpPkt. It contains property ID of target and property with requested values.
static const uint32_t XIPP_CMD_CHG = 0x0005;

//! \brief      Update notification sent regarding changed property values of sender.
//! \details    Sent as XippPrpPkt. It contains property ID of sender and updated property.
static const uint32_t XIPP_CMD_UPD = 0x0006;

//! \brief      Heartbeat command broadcasted by NIPs to indicate number of packets sent since last heartbeat.
//! \details    Count includes this heartbeat command as well.
//!             Sent as XippPrpPkt. It contains property ID of sender and updated property.
static const uint32_t XIPP_CMD_HRT = 0x0007;

//! \brief      Send Data command from receiver of data (reserved for future use)
//! \details    Receiver of data can be a host or another NIP, which sends data
//!             to NIP(s) to keep sending data of a given data stream.
//!             Sent as XippCmdPkt. It contains property ID of target.
//!             If property ID is 0 it means to keep sending data for multiple
//!             data streams, but disabled data streams are not enabled.
//!             If property ID is non-0 it means to enable specified data stream
//!             if not enabled and keep sending data for specified data stream.
static const uint32_t XIPP_CMD_SND = 0x0008;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Transport-Level Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// UDP transport
static const char XIPP_UDP_DEF_SUBNET[] = "192.168.42.0";   //!< Default UDP subnet for instrumentation network
static const uint32_t XIPP_UDP_PORT_PRC = 17453;             //!< UDP port used for traffic to processors
static const uint32_t XIPP_UDP_PORT_OPR = 17452;             //!< UDP port used for traffic to operators 

static const char XIPP_UDP_REC_FULL_ADDR[] = "192.168.42.128:17452";   //!< Default UDP subnet for instrumentation network

// Legacy Bank definitions used in Early Xipp Development
// (when multiple banks are enabled, they are sent in increasing order)
static const uint8_t XIPP_DATASTREAM_ID_BANK1 = 33;
static const uint8_t XIPP_DATASTREAM_ID_BANK2 = 37;
static const uint8_t XIPP_DATASTREAM_ID_BANK3 = 41;
static const uint8_t XIPP_DATASTREAM_ID_BANK4 = 45;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
// \addtogroup group1 Xipp Packet Definitions 
//  Xipp Packet Definitions
//
//  The Xipp protocol is basically a family of packet definitions for exchanging 
//  data and configuration.  Applications that use Xipp can use these definitions
//  internally for efficient caching and storage of Xipp information.
//
//  IMPORTANT: the Xipp data structures include C++ member functions when 
//  compiled under C++ that translate the member variables of the structure to 
//  and from generic machine-native types like unsigned int.  These functions
//  allow automatic endian-safe access to data structures depending on the host 
//  platform, as all Xipp network and file packets are little endian for 
//  multi-byte integer types.  The functions also include assert() statements
//  that perform range-checking under debug builds.
//
//  There are the following types of Xipp packets:\n
//  1) XippPkt    - Generalized Xipp packet structure\n
//  2) XippCmdPkt - Packets with a command to a specific target\n
//  3) XippPrpPkt - Packets with a command and property information related to the command\n
//  4) XippSegPkt - Packets with binary segment data from data output streams\n
//  5) XippGrpPkt - Packets with binary sample group data from data output streams\n
//  
//  Packets are declared with maximum size data fields (see XippPacket).
//  In the packet structures, headers are declared that are used by the packets.
//  Specific packet types are declared and sized according to their actual
//  payloads.  This allows customized property structures to be created by
//  developers and passed through the framework by typecasting.
//
//  The headers are (in generalized order of appearance in a Xipp packet):\n
//  1) XippPktHdr - the very compact, generalized header at the start of every Xipp packet\n
//  2) XippCmdHdr - header to pass a command within a Xipp packet\n
//  3) XippPrpHdr - header with name and size to frame a subsequent block of property information\n
//
//  The packets have variable length data payloads and this can make 
//  declaration and type-casting a bit tricky.  The data format depends on
//  properties of data stream that sent it.
//  For example, if Data Output Stream 3 of Module 2 of Processor 1
//  sends sample group packets, then the receiver should use the properties
//  of that data output to access the data in the correct format
//  when receiving packets from that data output.
*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define the packing alignment for structures as 1-byte (both VC++ and GCC)
// IMPORTANT: Don't forget to pop this at the bottom of the declarations;
#pragma pack(push,1)

//  
// Packet Header Definitions
//

//! \brief      Very compact, generalized header at the start of every Xipp packet.
//! \note       Packet and header lengths should be multiples of quadlets (1 quadlet = 4 bytes).
//!             Data length is not necessarily multiple of quadlets. In that case 
//!             data should contain additional info to specify data length and 
//!             0 padding should be added to packet.
//! \note       Use packet length when calculating and allocating memory space 
//!             for a packet or transferring packet. It is simpler than calculating
//!             exact length based on packet data and if definitions change, 
//!             length need not be recalculated.
//! \sa         XippPkt, XippCmdPkt, XippPrpPkt, XippSegPkt, XippGrpPkt
//! \sa         XippNumberOfEntities, XippPacketConstants,
//!             XIPP_DAT_SIZE_MAX
struct XippPktHdr
{
    uint32_t    timeStamp;              //!< Timestamps are sample time based, which can be converted to system clock time
    uint8_t     processorId;            //!< Processor ID of sender
    uint8_t     moduleId;               //!< Module ID of sender, or 0 if processor level configuration information
    uint8_t     dataId;                 //!< Datastream ID of sender, or 0 if packet contains configuration information
    uint8_t     length;                 //!< Length of packet in quadlets (4 byte (=32 bit) blocks, eg, length=5 -> 20 bytes)
};


//  
// Command Header Definitions
//

//! \brief      Header to pass a command within a Xipp packet.
//! \sa         XippCmdPkt, XippPrpPkt
//! \sa         XippPacketConstants, XIPP_CMD_SIZE_MAX
struct XippCmdHdr
{
    uint32_t    command;                //!< Command to execute
    uint8_t     processorId;            //!< Target processor ID, 0 if not specified 
    uint8_t     moduleId;               //!< Target module ID, 0 if not specified
    uint16_t    propertyId;             //!< Property index.
                                        //!< Its meaning depends on command parameter.
                                        //!< Its scope depends on processorId and moduleId parameters.
};


//  
// Property Header Definitions
//

//! \brief      Property header.
//! \sa         XippProperty, XIPP_CFG_SIZE_MAX
struct XippPrpHdr
{
    char        type[XIPP_PROPERTY_STRLEN]; //!< Configuration data type 
    char        name[XIPP_PROPERTY_STRLEN]; //!< Name of the property
    uint16_t    group;                      //!< Group where property belongs; 0: module level property; 
    uint32_t    flags;                      //!< Flags; bit 0: hidden flag
    uint16_t    cfgSize;                    //!< Size of the configuration data section in bytes
};


//  
//  Property Structure Definitions
//

//! \addtogroup group1
//! Properties consist of a header with the property name, size, and other info,
//! and a configuration (cfg) structure with the property data (see XippProperty).
//!
//! <i>Example 1:</i> to create a Range property with int32 min and max values
//! \li create a XippRangeCfg structure with { int min, max; }
//! \li define its type (e.g., "MyRangeCfg")
//! \li add access functions to XippRangeCfg to fill in variables
//!
//! <i>Example 2:</i> to use Range property
//! \li create a XippPacket buffer
//! \li cast it to a XippPrpPkt packet structure
//! \li fill in the type (e.g., "MyRangeCfg") and size (e.g., sizeof(XippRangeCfg))
//!     member variables of the prpHdr member
//! \li use XippRangeCfg access functions to access its members
//!
//! In this way, the configuration data in the XippProperty can be separated out.

//! \brief      Generic property structure for type casting
//! \sa         XippPrpHdr, XippPrpPkt
struct XippProperty
{   
    XippPrpHdr  prpHdr;                 //!< Property header
    uint8_t     cfg[];                  //!< Property data
};


//
//  Packet Data Field Size Limitations
// 
//  Declare them together here, as they will be needed for the packet structures.
//  NOTE: Review these definitions if any of the data packet structures are changed.
//

//! Max Packet Size selected for support by many transport layers and ability to capture data quadlet length in 8 bits
static const unsigned int XIPP_PKT_SIZE_MAX = 1024; 

//! Max data payload size (of any type) for a Xipp packet
static const unsigned int XIPP_DAT_SIZE_MAX = XIPP_PKT_SIZE_MAX - sizeof(XippPktHdr);

//! Max command data payload size for a command packet
static const unsigned int XIPP_CMD_SIZE_MAX = XIPP_DAT_SIZE_MAX - sizeof(XippCmdHdr); 

//! Max configuration data payload in a XippProperty structure
static const unsigned int XIPP_CFG_SIZE_MAX = XIPP_CMD_SIZE_MAX - sizeof(XippPrpHdr); 


//  
//  Packet Definitions
//

//! \brief      Structure to define a packet buffer.
//! \sa         XippPkt, XIPP_PKT_SIZE_MAX
typedef struct
{
    char        szPacket[XIPP_PKT_SIZE_MAX];    //!< Packet buffer.
} XippPacket;

//! \brief      The generalized Xipp packet structure.
//! \sa         XippPktHdr, XippPacket
struct XippPkt
{   
    XippPktHdr  pktHdr;         //!< Packet header
    uint8_t     data[];         //!< Data structure
};


//! \brief      Xipp Command packet
//! \sa         XippPktHdr, XippCmdHdr, XippPacket
struct XippCmdPkt
{
    XippPktHdr  pktHdr;                 //!< Packet header
    XippCmdHdr  cmdHdr;                 //!< Command header
    uint8_t     cmd[XIPP_CMD_SIZE_MAX]; //!< Command structure
};


//! \brief      Xipp Property Command packet
//! \sa         XippPktHdr, XippCmdHdr, XippProperty, XippPacket
struct XippPrpPkt
{
    XippPktHdr   pktHdr;        //!< Packet header
    XippCmdHdr   cmdHdr;        //!< Command header
    XippProperty property;      //!< Property structure
};


// General data format packets.

//! \brief      Xipp Segment packet
//! \sa         XippPktHdr, XippPacket
struct XippSegPkt
{
    XippPktHdr  pktHdr;             //!< Packet header
    uint8_t     classification;     //!< Classification of segment
    uint8_t     supplement;         //!< Reserved - supplemental information
    uint16_t    waveLength;         //!< Number of points in the wave
    int16_t     wave[];             //!< Segment wave data
};


//! \brief      Xipp Multi Channel Sample Group packet
//! \sa         XippPktHdr, XippPacket
struct XippGrpPkt
{
    XippPktHdr  pktHdr;             //!< Packet header
    int16_t     sample[];           //!< Data sample
};


//! \brief      Xipp Digital data packet
//! \sa         XippPktHdr, XippPacket
struct XippDigPkt
{
    XippPktHdr  pktHdr;             //!< Packet header
    uint16_t    flags;              //!< Flags - unspecified
    uint16_t    dataLength;         //!< Length of data field in bytes
    uint16_t    data[];             //!< Digital data
};


// Restore original byte packing
#pragma pack(pop)


#endif // XIPP_BASE_TYPES_H
