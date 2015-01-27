import
    "Router",
    "SystemQueue",
    "StandardHost",
    "FlatNetworkConfigurator";

channel ethernetline2
    delay 0.1us;
    datarate 100*1000000;
endchannel

channel gigabitline2
    delay 0.1us;
    datarate 1000*1000000;
endchannel

module AribaSim
    parameters:
        numrouters: numeric const,
        hostsperrouter: numeric const;
    submodules:
        systemqueue: SystemQueue;
            display: "i=block/queue";

        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router StandardHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "i=block/cogwheel";

        r: Router[numrouters];
            display: "i=abstract/router";
        node: StandardHost[numrouters*hostsperrouter];
            display: "i=device/laptop_vs";
    connections nocheck:
        for i=0..numrouters-1, j=0..hostsperrouter-1 do
            node[i*hostsperrouter+j].out++ --> ethernetline2 --> r[i].in++;
            node[i*hostsperrouter+j].in++ <-- ethernetline2 <-- r[i].out++;
        endfor

        for i=0..numrouters-2 do
            r[i].out++ --> gigabitline2 --> r[i+1].in++;
            r[i+1].out++ --> gigabitline2 --> r[i].in++;
        endfor
endmodule

network AribaSimulation : AribaSim
    parameters:
        numrouters = input(3, "Number of routers"),
        hostsperrouter = input(5, "Hosts per router");
endnetwork
