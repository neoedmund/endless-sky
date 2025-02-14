/* Messages.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

# include "Messages.h"

# include <mutex>

# include <textfrog.h>
extern textfrog tfg ;

using namespace std ;

namespace {
	mutex incomingMutex ;

	vector < pair < string , Messages :: Importance >> incoming ;
	vector < Messages :: Entry > list ;
}

// Add a message to the list along with its level of importance
void Messages :: Add ( const string & message , Importance importance ) {
//	printf ( "msg len:%d[%s]%p\n" , message . length ( ) , message . c_str ( ) , message . c_str ( ) ) ;
//	printf ( "tfg env=%p %d\n" , tfg , tfg -> env ) ;
//	tfg_call ( tfg , "mylog" , 1 , message . c_str ( ) ) ;
//	tfg_flushLog ( ) ;
	lock_guard < mutex > lock ( incomingMutex ) ;
	incoming . emplace_back ( message , importance ) ;
}

// Get the messages for the given game step. Any messages that are too old
// will be culled out, and new ones that have just been added will have
// their "step" set to the given value.
const vector < Messages :: Entry > & Messages :: Get ( int step ) {
	lock_guard < mutex > lock ( incomingMutex ) ;

	// Load the incoming messages.
	for ( const pair < string , Importance > & item : incoming ) {
		const string & message = item . first ;
		Importance importance = item . second ;

		// If this message is not important and it is already being shown in the
		// list, ignore it.
		if ( importance == Importance :: Low ) {
			bool skip = false ;
			for ( const Messages :: Entry & entry : list )
			skip |= ( entry . message == message ) ;
			if ( skip )
			continue ;
		}

		// For each incoming message, if it exactly matches an existing message,
		// replace that one with this new one.
		auto it = list . begin ( ) ;
		while ( it != list . end ( ) ) {
			// Each time a new message comes in, "age" all the existing ones to
			// limit how many of them appear at once.
			it -> step -= 60 ;
			// Also erase messages that have reached the end of their lifetime.
			if ( ( importance != Importance :: Low && it -> message == message ) || it -> step < step - 1000 )
			it = list . erase ( it ) ;
			else
			++ it ;
		}
		list . emplace_back ( step , message , importance ) ;
	}
	incoming . clear ( ) ;
	return list ;
}

// Reset the messages (i.e. because a new game was loaded).
void Messages :: Reset ( ) {
	lock_guard < mutex > lock ( incomingMutex ) ;
	incoming . clear ( ) ;
	list . clear ( ) ;
}
