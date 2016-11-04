void teamThinkTFC_Hunted( int team, int member )
{
	
	// normal bodyguard
	if (team==TFC_BODYGUARD && member==NORMAL) {

		// should it become a leader?
		int requiredLeaders = numTeamMembers() / 3;
		if ( numTeamLeaders() < requiredLeaders ) {
			becomeTeamLeader();
		}
		// is there any assigned task?
		else if ( assignedTask() ) {
			execAssignedTask();
		}
		// default: search and follow leader
		else {
			if (followValid()) followLeader();
			else searchForLeader();
		}
		return;
	}

	if (team==TFC_BODYGUARD && member==LEADER) {
		switch (state)
		case 1: if ( chosePathToEscapeDoor() ) state++;
				break;
		case 2: followPath();
				if ( escapeDoorReached() ) state++;
	}

	if (team==TFC_VIP) {
		switch (state)
		case 0:
}