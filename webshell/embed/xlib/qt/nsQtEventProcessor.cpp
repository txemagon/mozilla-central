/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is Mozilla Communicator client code.
 *
 * The Initial Developer of the Original Code is Netscape Communications
 * Corporation.  Portions created by Netscape are Copyright (C) 1998
 * Netscape Communications Corporation.  All Rights Reserved.
 * 
 * Contributor(s): 
 */


#include <stdio.h>
#include "nsQtEventProcessor.h"

nsQtEventProcessor::nsQtEventProcessor( nsIEventQueue* eventQueue, QObject* parent )
: QSocketNotifier( eventQueue->GetEventQueueSelectFD(), QSocketNotifier::Read, parent ), 
	m_eventQueue( eventQueue )
{
	printf("CONNETING SOCKET...\n");
	connect(this, SIGNAL(activated(int)), SLOT(handleActivated(int)));
	printf("DONE.\n");
}


nsQtEventProcessor::~nsQtEventProcessor( )
{
}


// private slot
void nsQtEventProcessor::handleActivated( int socket )
{
	printf("nsQtEventProcessor::handleActivated( %d )\n", socket);
	m_eventQueue->ProcessPendingEvents( );
}

