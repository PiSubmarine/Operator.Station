# Adding real PiSubmarine transports

- [x] Add "--grpc-server <host:port>" CLI option to set the remote gRPC server address.
- [x] Add "--telemetry-server <host:port>" CLI option to set the remote Telemetry server address.
- [x] Add "--control-server <host:port>" CLI option to set the remote Control server address.
- [x] Add "--video-bind <host:port>" CLI option to set the local endpoint for video streaming.
- [x] Add --fake-lease CLI option to enable fake lease provider.
- [x] Add --fake-telemetry CLI option to enable fake telemetry provider.
- [x] Add --fake-control CLI option to enable fake control input sink.
- [x] Use Telemetry.Client.Udp as real telemetry provider.
- [x] Use Video.Subscription.Grpc.Client as real video subscription client.
- [x] Use Control.Client.Udp as real control input client.
- [x] Use Grpc.Client to setup gRPC client.
- [x] Make sure that the above-mentioned providers are not blocking the UI thread. Use Controllers thread.
- [x] Use *.Telemetry.Protobuf deserializers for Lamp, Motor, Battery and other telemetry sub-domains. These deserializers are already available in GitHub and the project directory.
- [x] Add telemetry for Proximity sensor (Proximity.Telemetry.Api), Ballast (Ballast.Telemetry.Api), Video (Video.Telemetry.Api), Depth sensor (Depth.Telemetry.Api
  ).
- [x] Make sure to gracefully handle network errors. Temporary unavailability, reconnections must be handled.