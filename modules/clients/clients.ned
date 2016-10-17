package modules.clients;

simple client like IClient{
    parameters:
    	@display("i=abstract/people;is=l");
    	double lambda = default(1);
	double W = default(1); //TODO
	double check_time = default(0.1);
	double RTT = default(0.1);
    gates:
    	inout client_port;
}


