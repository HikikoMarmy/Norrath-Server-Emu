#include "../global_define.h"

#include "GameSessionManager.h"
#include "../Network/Event/NotifyClientDiscovered.h"
#include "../Network/Event/NotifyClientReqConnect.h"
#include "../Network/Event/NotifyGameDiscovered.h"

#include "../Network/Event/NotifyClientRequestConnect_RTA.h"

GameSessionManager::GameSessionManager()
{
	m_gameIndex = 0;
	m_gameSessionList.clear();
}

GameSessionManager::~GameSessionManager()
{
}

void GameSessionManager::OnDisconnectUser( sptr_user user )
{
	if( !user || user->m_gameId < 0 )
		return;

	const auto gameId = user->m_gameId;

	auto session = FindGame( gameId );
	if( !session )
		return;

	auto owner = session->m_owner.lock();
	if( !owner )
	{
		Log::Error( "Game session owner not found! [%d]", gameId );
		ForceTerminateGame( gameId );
		return;
	}

	if( owner->m_sessionId == user->m_sessionId )
	{
		Log::Info( "Game session owner disconnected! [%d]", gameId );
		ForceTerminateGame( gameId );
	}
}

bool GameSessionManager::CreatePublicGameSession( sptr_user owner, std::wstring gameName, RealmClientType clientType )
{
	auto new_session = std::make_shared< GameSession >();

	new_session->m_type = GameSession::GameType::Public;
	new_session->m_clientType = clientType;
	new_session->m_gameIndex = m_gameIndex;
	new_session->m_hostLocalAddr = L"";
	new_session->m_gameName = gameName;
	new_session->m_currentPlayers = 1;
	new_session->m_maximumPlayers = 4;

	new_session->m_gameData.resize( 256 );

	owner->m_isHost = true;
	owner->m_discoveryAddr = "";

	owner->m_gameId = m_gameIndex;
	new_session->m_owner = owner;

	std::lock_guard< std::mutex > lock( m_dataMutex );
	m_gameSessionList.push_back( new_session );

	m_gameIndex++;

	return true;
}

bool GameSessionManager::CreatePrivateGameSession( sptr_user owner, std::wstring gameName, RealmClientType clientType )
{
	// Check if the game name or host session id is already in use
	for( auto &gameSession : m_gameSessionList )
	{
		if( gameSession->m_type != GameSession::GameType::Private )
			continue;

		if( gameSession->m_gameName == gameName )
		{
			Log::Error( "Game name is already in use! [%S]", gameName.c_str() );
			return false;
		}
	}

	auto new_session = std::make_shared< GameSession >();

	new_session->m_type = GameSession::GameType::Private;
	new_session->m_clientType = clientType;
	new_session->m_gameIndex = m_gameIndex;
	new_session->m_hostLocalAddr = L"";
	new_session->m_gameName = gameName;
	new_session->m_currentPlayers = 1;
	new_session->m_maximumPlayers = 4;

	new_session->m_gameData.resize( 256 );

	owner->m_isHost = true;
	owner->m_discoveryAddr = "";

	owner->m_gameId = m_gameIndex;
	new_session->m_owner = owner;

	std::lock_guard< std::mutex > lock( m_dataMutex );
	m_gameSessionList.push_back( new_session );

	m_gameIndex++;

	return true;
}

bool GameSessionManager::ForceTerminateGame( int32_t gameId )
{
	if( gameId < 0 )
	{
		return false;
	}

	std::lock_guard< std::mutex > lock( m_dataMutex );

	auto it = std::find_if( m_gameSessionList.begin(), m_gameSessionList.end(), [ &gameId ]( sptr_game_session gameSession )
	{
		return gameSession->m_gameIndex == gameId;
	} );

	if( it != m_gameSessionList.end() )
	{
		m_gameSessionList.erase( it );
		return true;
	}

	return false;
}

sptr_game_session GameSessionManager::FindGame( const int32_t gameId )
{
	if( gameId < 0 ) return nullptr;

	std::lock_guard< std::mutex > lock( m_dataMutex );

	for( auto &gameSession : m_gameSessionList )
	{
		if( gameSession->m_gameIndex == gameId )
		{
			return gameSession;
		}
	}

	return nullptr;
}

sptr_game_session GameSessionManager::FindGame( const std::wstring &gameName )
{
	if( gameName.empty() ) return nullptr;

	std::lock_guard< std::mutex > lock( m_dataMutex );

	for( auto &gameSession : m_gameSessionList )
	{
		if( gameSession->m_gameName == gameName )
		{
			return gameSession;
		}
	}

	return nullptr;
}

bool GameSessionManager::RequestOpen( sptr_user user )
{
	auto gameId = user->m_gameId;
	auto session = FindGame( gameId );

	if( session == nullptr )
	{
		Log::Error( "Game session not found! [%d]", gameId );
		return false;
	}

	if( session->m_state == GameSession::GameState::Open )
	{
		return false;
	}

	if( user->m_discoveryAddr.empty() )
	{
		Log::Error( "User discovery address is empty! [%d]", gameId );
		return false;
	}

	// Very cool that they couldn't agree on ASCII or UTF-16
	session->m_hostLocalAddr = std::wstring( user->m_localAddr.begin(), user->m_localAddr.end() );
	session->m_hostExternalAddr = std::wstring( user->m_localAddr.begin(), user->m_localAddr.end() );
	session->m_hostPort = user->m_discoveryPort;

	session->m_state = GameSession::GameState::Open;

	// Tell the host its own address.
	NotifyGameDiscovered msg( user->m_discoveryAddr, user->m_discoveryPort, user->m_clientType );
	user->sock->send( msg );

	Log::Info( "Game Session [%d] Discoverable on %s", gameId, user->m_discoveryAddr.c_str() );

	return true;
}

bool GameSessionManager::RequestCancel( sptr_user user )
{
	if( !user || user->m_gameId < 0 )
		return false;

	const int gameId = user->m_gameId;

	std::lock_guard<std::mutex> lock( m_dataMutex );

	auto it = std::find_if( m_gameSessionList.begin(), m_gameSessionList.end(),
							[ gameId ]( const sptr_game_session &gameSession )
	{
		return gameSession->m_gameIndex == gameId;
	} );

	if( it == m_gameSessionList.end() )
		return false;

	const auto &session = *it;

	auto owner = session->m_owner.lock();
	if( !owner )
	{
		Log::Error( "Game session owner not found! [%d]", gameId );
		ForceTerminateGame( gameId );
		return false;
	}

	if( owner->m_sessionId != user->m_sessionId )
	{
		Log::Error( "User is not the host! [%d]", gameId );
		return false;
	}

	m_gameSessionList.erase( it );
	return true;
}

bool GameSessionManager::RequestJoin( sptr_user join_user )
{
	const auto gameId = join_user->m_gameId;
	auto session = FindGame( gameId );

	if( session == nullptr )
	{
		Log::Error( "Game session not found! [%d]", gameId );
		return false;
	}

	if( session->m_state != GameSession::GameState::Open )
	{
		Log::Error( "Game session not open! [%d]", gameId );
		return false;
	}

	auto host_user = session->m_owner.lock();

	if( host_user == nullptr )
	{
		Log::Error( "Host not found! [%d]", gameId );
		ForceTerminateGame( gameId );
		return false;
	}

	if( host_user->m_discoveryAddr.empty() )
	{
		Log::Error( "User discovery address is empty! [%d]", gameId );
		ForceTerminateGame( gameId );
		return false;
	}

	join_user->m_isHost = false;

	// Tell the joiner its own address. 
	NotifyClientDiscovered msgClientDiscovered( host_user->m_discoveryAddr, host_user->m_discoveryPort, host_user->m_clientType );
	join_user->sock->send( msgClientDiscovered );

	// Notify the host that a client is trying to connect.
	if( host_user->m_clientType == RealmClientType::CHAMPIONS_OF_NORRATH )
	{
		NotifyClientRequestConnect msgNotifyReqConnect(
			join_user->m_discoveryAddr,
			join_user->m_discoveryPort
		);

		host_user->sock->send( msgNotifyReqConnect );
	}
	else
	{
		NotifyClientRequestConnect_RTA msgNotifyReqConnect(
			join_user->m_discoveryAddr,
			join_user->m_localAddr,
			join_user->m_discoveryPort
		);

		host_user->sock->send( msgNotifyReqConnect );
	}
	
	Log::Info( "User [%S] Joining game session... [%d]", join_user->m_sessionId.c_str(), gameId );

	return true;
}

std::vector<sptr_game_session> GameSessionManager::GetAvailableGameSessionList( RealmClientType clientType ) const
{
	std::lock_guard<std::mutex> lock( m_dataMutex );

	std::vector<sptr_game_session> list;
	for( const auto &game : m_gameSessionList )
	{
		if( game->m_clientType != clientType )
			continue;

		if( game->m_type == GameSession::GameType::Public &&
			game->m_state == GameSession::GameState::Open )
		{
			list.push_back( game );
		}
	}
	return list;
}

std::vector<sptr_game_session> GameSessionManager::GetPublicGameSessionList( RealmClientType clientType ) const
{
	std::lock_guard<std::mutex> lock( m_dataMutex );

	std::vector<sptr_game_session> list;
	for( const auto &game : m_gameSessionList )
	{
		if( game->m_clientType != clientType )
			continue;

		if( game->m_type == GameSession::GameType::Public )
			list.push_back( game );
	}
	return list;
}

std::vector<sptr_game_session> GameSessionManager::GetPrivateGameSessionList( RealmClientType clientType ) const
{
	std::lock_guard<std::mutex> lock( m_dataMutex );

	std::vector<sptr_game_session> list;
	for( const auto &game : m_gameSessionList )
	{
		if( game->m_clientType != clientType )
			continue;

		if( game->m_type == GameSession::GameType::Private )
			list.push_back( game );
	}
	return list;
}