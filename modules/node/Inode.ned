package modules.node;


moduleinterface Inode
{
    parameters:
	string RS;
	string FS;

	int level;
	double betweenness;

    gates:
	inout face[];	
	inout client_port;

}

