#pragma once
#include <memory>

#include "base/async_task.h"
#include "real_audio_common.h"
#include "protobuf_packet.h"
#include "user_service.pb.h"
#include "base/tcp_socket.h"
namespace audio_engine{
#define DEFAULT_SEND
	typedef int socket_t;
	typedef std::shared_ptr<audio_engine::RAUserMessage> RAUserMessagePtr;
	class UserManager;
	class UserConnection;
	typedef std::shared_ptr<UserConnection> UserConnPtr;
	class UserConnection : public std::enable_shared_from_this<UserConnection>
	{
	public:
		UserConnection( UserManager* host );
		UserConnection(TcpSocketPtr tcp);
		void SetVerifyAccountCB( std::function<int(RAUserMessagePtr pb, UserConnPtr conn)> cb);
		~UserConnection();
		void AttachTcp( TcpSocketPtr tcp );
		void DettachTcp();
		std::string userid();
		int64_t token();
		std::string extend();
		audio_engine::DEVICE_TYPE device_type();
		int state() { return _state; }
		void set_extend( std::string extend ) { _extend = extend; }
		void set_state( int state ) { _state = state; }
	public:
		void RecvPacket( std::error_code ec, std::shared_ptr< audio_engine::RAUserMessage> pb );
		void VerifyFailed();
		void Send( int type, BufferPtr buf );
	private:
		void Read();
		void Write( int type, BufferPtr buf );
		void HandleError( std::error_code ec );
		void HandleLogin( const audio_engine::RequestLogin& login_req,int sn );
		void HandleLogout( const ::audio_engine::RequestLogout& logout_req,int sn );
		std::function<int( RAUserMessagePtr pb, UserConnPtr conn )> _VerifyAccount;
		std::string _userid;
		std::string _extend;
		int64_t _token = 0;
		audio_engine::DEVICE_TYPE _device_type = audio_engine::DEVICE_UNKNOWN;
		int _state = audio_engine::STATE_INVALID;
		BufferPool* _buffer_pool = nullptr;
		AsyncTask* _task = nullptr;
		TcpSocketPtr _tcp_socket;
		ProtoPacket _proto_packet;
		UserManager* _host;
		bool _first_packet = true;
		bool _stop = false;
	};

}