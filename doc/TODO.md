# Adding real PiSubmarine transports

- [ ] Add "--grpc-server <host:port>" CLI option to set the remote gRPC server address.
- [ ] Add "--telemetry-server <host:port>" CLI option to set the remote Telemetry server address.
- [ ] Add "--control-server <host:port>" CLI option to set the remote Control server address.
- [ ] Add "--video-bind <host:port>" CLI option to set the local endpoint for video streaming.
- [ ] Add --fake-lease CLI option to enable fake lease provider.
- [ ] Add --fake-telemetry CLI option to enable fake telemetry provider.
- [ ] Add --fake-control CLI option to enable fake control input sink.
- [ ] Use Telemetry.Client.Udp as real telemetry provider.
- [ ] Use Video.Subscription.Grpc.Client as real video subscription client.
- [ ] Use Control.Client.Udp as real control input client.
- [ ] Use Grpc.Client to setup gRPC client.
- [ ] Make sure that the above-mentioned providers are not blocking the UI thread. Use Controllers thread.
- [ ] Use *.Telemetry.Protobuf deserializers for Lamp, Motor, Battery and other telemetry sub-domains. These deserializers are already available in GitHub and the project directory.
- [ ] Add telemetry for Proximity sensor (Proximity.Telemetry.Api), Ballast (Ballast.Telemetry.Api), Video (Video.Telemetry.Api), Depth sensor (Depth.Telemetry.Api
  ).
- [ ] Make sure to gracefully handle network errors. Temporary unavailability, reconnections must be handled.