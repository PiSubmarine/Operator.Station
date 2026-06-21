# Input system

- [ ] Create Operator::Station::Input::CaptureController class to create IAxis and IKey instances using Input.Api.IManager
- [ ] Create Operator::Station::Input::BindingController class to create and edit Binding with Input::Api::IBinder, and store/load bindings with Input::Api::ISerializer
- [ ] Create Operator::Station::Input::View::BindingViewModel and related QML items to display and edit bindings.
- [ ] Remove Surge, Yaw and Ballast from Control ViewModel
- [ ] Decide whether to provide IAxis and IKey to Control::Controller or provide captured raw values instead.
