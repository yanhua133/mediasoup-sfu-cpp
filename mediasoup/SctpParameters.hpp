#pragma once
#include <string>
#include <vector>

using json = nlohmann::json;
namespace mediasoup
{
  

/**
 * Both OS and MIS are part of the SCTP INIT+ACK handshake. OS refers to the
 * initial number of outgoing SCTP streams that the server side transport creates
 * (to be used by DataConsumers), while MIS refers to the maximum number of
 * incoming SCTP streams that the server side transport can receive (to be used
 * by DataProducers). So, if the server side transport will just be used to
 * create data producers (but no data consumers), OS can be low (~1). However,
 * if data consumers are desired on the server side transport, OS must have a
 * proper value and such a proper value depends on whether the remote endpoint
 * supports  SCTP_ADD_STREAMS extension or not.
 *
 * libwebrtc (Chrome, Safari, etc) does not enable SCTP_ADD_STREAMS so, if data
 * consumers are required,  OS should be 1024 (the maximum number of DataChannels
 * that libwebrtc enables).
 *
 * Firefox does enable SCTP_ADD_STREAMS so, if data consumers are required, OS
 * can be lower (16 for instance). The mediasoup transport will allocate and
 * announce more outgoing SCTM streams when needed.
 *
 * mediasoup-client provides specific per browser/version OS and MIS values via
 * the device.sctpCapabilities getter.
 */
struct NumSctpStreams
{
	/**
	 * Initially requested number of outgoing SCTP streams.
	 */
	int OS = 1024;

	/**
	 * Maximum number of incoming SCTP streams.
	 */
	int MIS = 1024;
};

struct SctpParameters
{
	/**
	 * Must always equal 5000.
	 */
	int port = 0;

	/**
	 * Initially requested number of outgoing SCTP streams.
	 */
	int OS = 0;

	/**
	 * Maximum number of incoming SCTP streams.
	 */
	int MIS =0 ;

	/**
	 * Maximum allowed size for SCTP messages.
	 */
	int maxMessageSize = 0 ;
};

/**
 * SCTP stream parameters describe the reliability of a certain SCTP stream.
 * If ordered is true then maxPacketLifeTime and maxRetransmits must be
 * false.
 * If ordered if false, only one of maxPacketLifeTime or maxRetransmits
 * can be true.
 */
struct SctpStreamParameters
{
	/**
	 * SCTP stream id.
	 */
	int streamId = 0;

	/**
	 * Whether data messages must be received in order. If true the messages will
	 * be sent reliably. Default true.
	 */
	bool ordered =false;

	/**
	 * When ordered is false indicates the time (in milliseconds) after which a
	 * SCTP packet will stop being retransmitted.
	 */
	int maxPacketLifeTime = 0;

	/**
	 * When ordered is false indicates the maximum number of times a packet will
	 * be retransmitted.
	 */
	int maxRetransmits =0 ;
};
void to_json(json& j, const SctpStreamParameters& st);
void from_json(const json& j, SctpStreamParameters& st);
struct SctpCapabilities
{
  NumSctpStreams numStreams;
};


void to_json(json& j, const NumSctpStreams& st);
void from_json(const json& j, NumSctpStreams& st);

void to_json(json& j, const SctpParameters& st);
void from_json(const json& j, SctpParameters& st);

void to_json(json& j, const SctpCapabilities& st);
void from_json(const json& j, SctpCapabilities& st);
}
