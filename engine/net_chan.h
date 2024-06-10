#ifndef ENGINE_NET_CHAN_H
#define ENGINE_NET_CHAN_H

typedef enum netsrc_s
{
	NS_CLIENT = 0,
	NS_SERVER = 1,
	NS_MULTICAST = 2
} netsrc_t;

typedef struct netchan_s
{
	//TODO: implement - Solokiller
	/*
	netsrc_t sock;
	netadr_t remote_address;
	int player_slot;
	float last_received;
	float connect_time;
	double rate;
	double cleartime;
	int incoming_sequence;
	int incoming_acknowledged;
	int incoming_reliable_acknowledged;
	int incoming_reliable_sequence;
	int outgoing_sequence;
	int reliable_sequence;
	int last_reliable_sequence;
	void *connection_status;
	int( *pfnNetchan_Blocksize )( void * );
	sizebuf_t message;
	byte message_buf[ 3990 ];
	int reliable_length;
	byte reliable_buf[ 3990 ];
	fragbufwaiting_t *waitlist[ 2 ];
	int reliable_fragment[ 2 ];
	unsigned int reliable_fragid[ 2 ];
	fragbuf_t *fragbufs[ 2 ];
	int fragbufcount[ 2 ];
	__int16 frag_startpos[ 2 ];
	__int16 frag_length[ 2 ];
	fragbuf_t *incomingbufs[ 2 ];
	qboolean incomingready[ 2 ];
	char incomingfilename[ 260 ];
	void *tempbuffer;
	int tempbuffersize;
	flow_t flow[ 2 ];
	*/
} netchan_t;

extern char gDownloadFile[ 256 ];

void Netchan_Init();

void Netchan_Transmit( netchan_t* chan, int length, byte* data );

void NET_SendPacket( netsrc_t sock, int length, void* data, netadr_t to );

#endif //ENGINE_NET_CHAN_H
