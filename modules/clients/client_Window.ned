package modules.clients;

simple client_Window extends client{
    parameters:
         @class(client_Window);
                int defWinSize = default(1);
                int maxWinSize = default(10);
		double dropProb = default(0.005);
}



