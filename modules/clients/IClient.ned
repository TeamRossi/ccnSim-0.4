package modules.clients;

moduleinterface IClient {
    parameters:
		double lambda;
		double W;
		double check_time;
		double RTT;
    gates:
        inout client_port;
}




