"use strict";

const aedes = require("aedes");
const mqemitter = require("mqemitter");
const persistence = require("aedes-persistence");

const brokerPort = 4883;

function startAedes() {
  const broker = aedes({
    mq: mqemitter({
      concurrency: 100,
    }),
    persistence: persistence(),
    preConnect: function (client, packet, done) {
      console.log("Aedes preConnect check client ip:", client.connDetails);
      if (client.connDetails && client.connDetails.ipAddress) {
        client.ip = client.connDetails.ipAddress;
      }
      client.close();
      return done(null, true);
    },
  });

  const server = require("net").createServer(broker.handle);

  server.listen(process.env.PORT || brokerPort, "0.0.0.0", function () {
    console.log("Aedes listening on :", server.address());
    broker.publish({ topic: "aedes/hello", payload: "I'm broker " + broker.id });
  });

  broker.on("subscribe", function (subscriptions, client) {
    console.log(
      "MQTT client \x1b[32m" +
        (client ? client.id : client) +
        "\x1b[0m subscribed to topics: " +
        subscriptions.map((s) => s.topic).join("\n"),
      "from broker",
      broker.id
    );
  });

  broker.on("unsubscribe", function (subscriptions, client) {
    console.log(
      "MQTT client \x1b[32m" + (client ? client.id : client) + "\x1b[0m unsubscribed to topics: " + subscriptions.join("\n"),
      "from broker",
      broker.id
    );
  });

  // fired when a client connects
  broker.on("client", function (client) {
    console.log(
      "Client Connected: \x1b[33m" + (client ? client.id : client) + " ip  " + (client ? client.ip : null) + "\x1b[0m",
      "to broker",
      broker.id
    );
  });

  // fired when a client disconnects
  broker.on("clientDisconnect", function (client) {
    console.log("Client Disconnected: \x1b[31m" + (client ? client.id : client) + "\x1b[0m", "to broker", broker.id);
  });

  // fired when a message is published
  //broker.on("publish", async function (packet, client) {});
}

startAedes();
