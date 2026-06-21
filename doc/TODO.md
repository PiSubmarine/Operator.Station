# Connecting Operator.Station to real PiSubmarine transports

## Current baseline

- [x] Video already runs on a dedicated controllers thread in `main.cpp`.
- [x] Lease calls already run on a dedicated worker thread via `Lease::ThreadWorker` and `Lease::SyncLeaseIssuerProxy`.
- [x] Video transport configuration uses `--video-bind <host:port>`.
- [x] Video can already use a real RTP receive pipeline when `--fake-video` is not set.
- [x] Video subscription is selected in the composition root instead of being wired directly in `main.cpp`.
- [x] Replace the local `LocalVideoSubscriptionService` stub in `main.cpp`.

## CLI and configuration

- [x] Replace the current split video CLI with the target transport-oriented options:
  `--grpc-server <host:port>`, `--telemetry-server <host:port>`, `--control-server <host:port>`, `--video-bind <host:port>`.
- [x] Add `--tickrate <duration>` CLI option for the controllers-thread tick period, with default `10ms`.
- [x] Keep fake toggles for composition-root testing:
  `--fake-lease`, `--fake-telemetry`, `--fake-control`, `--fake-video`.
- [ ] Parse `host:port` values into explicit endpoint types instead of passing raw strings through `main.cpp`.
- [x] Reuse `--grpc-server` for all gRPC services.

## Dependencies and composition root

- [x] Add missing PiSubmarine dependencies to `app/CMakeLists.txt`:
  `Lease.Client.Grpc`, `Grpc.Client`, `Telemetry.Client.Udp`, `Control.Client.Udp`,
  `Battery.Telemetry.Protobuf`, `Lamp.Telemetry.Protobuf`, `Motor.Telemetry.Protobuf`,
  `Depth.Telemetry.Protobuf`, `Ballast.Telemetry.Protobuf`, `Proximity.Telemetry.Protobuf`,
  `Video.Telemetry.Protobuf`.
- [x] Add the currently required telemetry transport/security dependencies to `app/CMakeLists.txt`:
  `Telemetry.Client.Udp`, `Battery.Telemetry.Protobuf`,
  `Motor.Telemetry.Protobuf`, `Security.Aead.Openssl`, `Security.Nonce.Openssl`, `Udp.Asio`.
- [ ] Add any required transport/security dependencies pulled in by the real clients if they are not already reachable through transitive links.
- [x] Replace fake/local objects in `main.cpp` with real composition-root wiring behind CLI switches.

## Lease transport

- [x] Create a real gRPC channel using `PiSubmarine.Grpc.Client`.
- [x] Replace `Lease::FakeIssuer` with `Lease::Client::Grpc::Client` when `--fake-lease` is not set.
- [x] Keep the existing `Lease::ThreadWorker` + `Lease::SyncLeaseIssuerProxy` boundary so blocking transport work stays off the UI thread.

## Video transport

- [x] Replace `LocalVideoSubscriptionService` with `Video.Subscription.Grpc.Client::Client`.
- [x] Feed the gRPC video subscription client from the real `--grpc-server` endpoint.
- [x] Keep the existing RTP pipeline builder path for actual media reception.
- [x] Rename the existing video bind CLI to the final `--video-bind <host:port>` shape.

## Control transport

- [x] Replace `Control::FakeSink` with `Control.Client.Udp::Client` when `--fake-control` is not set.
- [x] Add the required serializer, nonce provider, AEAD provider, and UDP sender in the composition root.
- [x] Remove lease management from `Control::Controller`; `Control.Client.Udp::Client` must remain the only control lease owner.
- [ ] Adjust `Control.Client.Udp::Client` to treat `ErrorCondition::NotReady` as in-progress lease work, matching `Telemetry.Client.Udp`.
- [ ] Make sure `Control::Controller` keeps reporting submit failures clearly when the UDP client is temporarily unavailable.

## Telemetry transport

- [x] Introduce `Telemetry.Client.Udp::Client` in the composition root.
- [x] Replace the temporary telemetry endpoint assumption of `127.0.0.1:--telemetry-port` with the final `--telemetry-server <host:port>` option.
- [x] Create `Telemetry.Channels.Api` and move shared telemetry channel ids there.
- [x] Add per-channel `Telemetry.Client.Udp::Source` adapters for battery and motor domains consumed by the UI.
- [x] Replace the current battery/motor copied channel-id strings in `Operator.Station` and `Drone.Server.Fake` with `Telemetry.Channels.Api`.
- [x] Replace fake telemetry providers with protobuf deserializers for all available UI domains.
- [x] Replace fake battery and motor telemetry providers with protobuf deserializers:
  `Motor.Telemetry.Protobuf::Deserializer`,
  `Battery.Telemetry.Protobuf::Deserializer`.
- [x] Replace the lamp fake provider with `Lamp.Telemetry.Protobuf::Deserializer` once a shared lamp telemetry channel is defined and produced by the server.
- [x] Add new UI telemetry paths for:
  `Proximity.Telemetry.Api`,
  `Ballast.Telemetry.Api`,
  `Video.Telemetry.Api`,
  `Depth.Telemetry.Api`.
- [x] Replace private channel-id definitions in producers/consumers with `Telemetry.Channels.Api`
  for ids such as `battery.main`, `motor.front-left`, `motor.front-right`, `motor.rear-left`, `motor.rear-right`.
- [x] Remove lease handling from `Telemetry::Controller`.
- [x] Make `Telemetry::Controller` a pure UI-facing refresh coordinator for telemetry providers/deserializers.
- [x] Remove the direct `telemetry-main` lease ownership from `Operator.Station`; telemetry lease/subscription lifecycle must live only inside `Telemetry.Client.Udp::Client`.

## Resilience and lifecycle

- [ ] Treat temporary network failures as retryable for lease, telemetry, control, and video subscription paths.
- [ ] Verify reconnect behavior after lease loss, server restart, and UDP interruption.
- [ ] Add a deterministic controllers-thread tick loop based on `PiSubmarine.Time.Manager`, not independent `QTimer`s.
- [ ] Register tick-driven controller-thread objects in a fixed order so controller behavior stays deterministic under load.
- [ ] Keep the controllers-thread tick cadence fixed to the configured period even when individual ticks overrun.
- [ ] Review shutdown flow in `main.cpp`; controller destruction is still noted there as inconsistent.
- [ ] Ensure all real transport objects are created, used, and destroyed on the intended worker threads.

## Tests

- [ ] Add unit tests for CLI parsing of the new endpoint-shaped options.
- [ ] Add unit tests for `--tickrate` parsing and validation.
- [ ] Add unit tests for telemetry channel-to-deserializer wiring.
- [ ] Add unit tests for composition-root selection between fake and real providers.
- [ ] Add regression tests for reconnect/retry behavior where the current abstractions allow it.
- [ ] Verify that controller-layer tick processing continues when any individual domain controller reports an error.
- [ ] Add user-visible error propagation from controllers to ViewModels via signals and represented error state.
