import 'dart:async';
import 'package:async/async.dart';
import 'dart:convert';

import 'package:flutter_blue/flutter_blue.dart';
import 'package:flutter/material.dart';
import 'package:flutter_colorpicker/flutter_colorpicker.dart';

void main() => runApp(MyApp());

const BT_NUS_SERVICE_UUID = '6E400001-B5A3-F393-E0A9-E50E24DCCA9E';
const BT_RX_CHARACTERISTIC_UUID = '6E400002-B5A3-F393-E0A9-E50E24DCCA9E';
const BT_TX_CHARACTERISTIC_UUID = '6E400003-B5A3-F393-E0A9-E50E24DCCA9E';

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    var title = 'BLE RGB Lamp';
    return MaterialApp(
      title: title,
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: StreamBuilder<BluetoothState>(
          stream: FlutterBlue.instance.state,
          initialData: BluetoothState.unknown,
          builder: (c, snapshot) {
            final state = snapshot.data;
            if (state == BluetoothState.on) {
              return MyHomePage(title: title);
            }
            return BluetoothOffScreen(state: state);
          }),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key, this.title}) : super(key: key);

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  bool _scanning = false;
  Stream<List<BluetoothDevice>> connectedDevicesStream = Stream.empty();
  Stream<List<BluetoothDevice>> scannedDevicesStream = Stream.empty();
  String deviceIdConnecting;
  final keyState = new GlobalKey<ScaffoldState>();

  @override
  void initState() {
    super.initState();

    connectedDevicesStream = Stream.periodic(Duration(seconds: 2))
        .asyncMap((_) => FlutterBlue.instance.connectedDevices);

    scannedDevicesStream = FlutterBlue.instance.scanResults.asyncMap(
        (scanResultList) =>
            scanResultList.map((scanResult) => scanResult.device).toList());

    startScan();
  }

  startScan() {
    if (_scanning) {
      stopScan();
    }

    setState(() {
      _scanning = true;
      deviceIdConnecting = null;
    });

    FlutterBlue.instance.startScan(timeout: Duration(seconds: 5));
  }

  stopScan() {
    FlutterBlue.instance.stopScan();
  }

  /*
   * Super ugly workaround because we can't try/catch the connect method
   */
  Future<bool> connect(BluetoothDevice device) async {
    Future<bool> success;
    await device.connect().timeout(Duration(seconds: 5), onTimeout: () {
      success = Future.value(false);
      device.disconnect();
    }).then((data) {
      if (success == null) {
        success = Future.value(true);
      }
    });
    return success;
  }

  Future<void> onDeviceSelected(
      BuildContext context, BluetoothDevice device) async {
    setState(() {
      deviceIdConnecting = device.id.toString();
    });

    try {
      print('checking state');
      var state = await device.state.first;
      print(state);
      if (state != BluetoothDeviceState.connected) {
        print('not connected');
        var success = await connect(device);
        if (!success) {
          keyState.currentState.showSnackBar(SnackBar(
            content: Text("Failed to Connect"),
          ));
          setState(() {
            deviceIdConnecting = null;
          });
          return;
        }
      }
      print('Connect');
    } finally {
      setState(() {
        deviceIdConnecting = null;
      });
    }

    setState(() {
      deviceIdConnecting = null;
    });

    Navigator.of(context).push(
      MaterialPageRoute(builder: (context) => DeviceScreen(device: device)),
    );
  }

  Widget buildDeviceListFromStream(
      BuildContext context, Stream<List<BluetoothDevice>> stream) {
    return StreamBuilder<List<BluetoothDevice>>(
      stream: stream,
      initialData: [],
      builder: (c, snapshot) => Column(
        children: snapshot.data.map((device) {
          var isConnecting = deviceIdConnecting == device.id.toString();
          return ListTile(
            title: Text(device.name.isEmpty ? "No name" : device.name),
            subtitle: Text(device.id.toString()),
            dense: false,
            trailing: StreamBuilder<BluetoothDeviceState>(
              stream: device.state,
              initialData: BluetoothDeviceState.disconnected,
              builder: (c, snapshot) {
                if (isConnecting) {
                  return CircularProgressIndicator(
                    strokeWidth: 1,
                  );
                }
                if (snapshot.data == BluetoothDeviceState.connected) {
                  return Text('Connected');
                }
                return Text('Not Connected');
              },
            ),
            leading: Icon(Icons.bluetooth),
            onTap: () {
              if (deviceIdConnecting == null) {
                onDeviceSelected(context, device);
              }
            },
          );
        }).toList(),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      key: keyState,
      appBar: AppBar(
        title: Text(widget.title),
      ),
      body: SingleChildScrollView(
        child: Column(
          children: <Widget>[
            buildDeviceListFromStream(context, connectedDevicesStream),
            buildDeviceListFromStream(context, scannedDevicesStream),
          ],
        ),
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: startScan,
        tooltip: 'Refresh',
        child: Icon(Icons.refresh),
      ),
    );
  }
}

class DeviceScreen extends StatefulWidget {
  const DeviceScreen({Key key, this.device}) : super(key: key);

  final BluetoothDevice device;

  @override
  _DeviceScreenState createState() => _DeviceScreenState();
}

class _DeviceScreenState extends State<DeviceScreen> {
  Timer timer;
  Color pickerColor = Color(0xFFFFFF);
  BluetoothCharacteristic rxChar;
  BluetoothCharacteristic txChar;
  StreamSubscription<BluetoothDeviceState> stateSub;
  StreamSubscription<List<int>> txSub;

  @override
  void initState() {
    super.initState();
    new Future.delayed(Duration.zero, () {
      fetchServices(this.context);
    });
  }

  @override
  void deactivate() {
    super.deactivate();
    onDisconnect();
  }

  onDisconnect() {
    if (stateSub != null) {
      stateSub.cancel();
      stateSub = null;
    }

    if (txSub != null) {
      txSub.cancel();
      txSub = null;
    }

    widget.device.disconnect();
  }

  fetchServices(BuildContext context) async {
    print('fetch services');
    var services = await widget.device.discoverServices();
    if (stateSub != null) {
      stateSub.cancel();
      stateSub = null;
    }

    stateSub = widget.device.state.listen((state) {
      if (state == BluetoothDeviceState.disconnected) {
        onDisconnect();
      }
    });

    var uartService = services.firstWhere(
        (service) =>
            service.uuid.toString().toUpperCase() == BT_NUS_SERVICE_UUID,
        orElse: () => null);

    if (uartService == null) {
      print("UART Service not found");
      return;
    }

    rxChar = uartService.characteristics.firstWhere(
        (char) =>
            char.uuid.toString().toUpperCase() == BT_RX_CHARACTERISTIC_UUID,
        orElse: () => null);

    txChar = uartService.characteristics.firstWhere(
        (char) =>
            char.uuid.toString().toUpperCase() == BT_TX_CHARACTERISTIC_UUID,
        orElse: () => null);

    try {
      var active = await txChar.setNotifyValue(true);
      await txChar.read();
      print('Active ?');
      print(active);
    } catch (err) {}
    if (txSub != null) {
      txSub.cancel();
      txSub = null;
    }
    txSub = txChar.value.listen((value) {
      print("New value");
      print(value);
      var json = utf8.decode(value);
      print(json);
    });
  }

  void writeData(Color color) async {
    try {
      var colorStr =
          color.value.toRadixString(16).padLeft(8, '0').substring(2, 8);
      var obj = {'color': colorStr};
      var cmd = jsonEncode(obj);
      List<int> bytes = utf8.encode(cmd);
      print(cmd);
      await rxChar.write(bytes, withoutResponse: true);
      timer = null;
    } on Exception catch (err) {
      print(err);
    }
  }

  void onColorChange(BuildContext context, Color color) async {
    setState(() => pickerColor = color);

    if (rxChar == null) {
      await fetchServices(context);
    }

    if (timer == null) {
      timer = Timer(Duration(milliseconds: 50), () => writeData(color));
    }
  }

  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.device.name),
      ),
      body: Center(
        child: Padding(
          padding: EdgeInsets.only(top: 44),
          child: Column(
            children: <Widget>[
              /*StreamBuilder<int>(
              stream: widget.device.mtu,
              initialData: 0,
              builder: (c, snapshot) => Text('${snapshot.data} bytes'),
            )*/
              ColorPicker(
                showLabel: true,
                enableAlpha: false,
                onColorChanged: (color) => onColorChange(context, color),
                pickerColor: pickerColor,
              ),
            ],
          ),
        ),
      ),
    );
  }
}

class BluetoothOffScreen extends StatelessWidget {
  const BluetoothOffScreen({Key key, this.state}) : super(key: key);

  final BluetoothState state;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Colors.lightBlue,
      appBar: AppBar(
        title: Text("Bluetooth Off"),
      ),
      body: Center(
        child: Column(
          mainAxisSize: MainAxisSize.min,
          children: <Widget>[
            Icon(
              Icons.bluetooth_disabled,
              size: 200.0,
              color: Colors.white54,
            ),
            Text(
              'Bluetooth Adapter is ${state != null ? state.toString().substring(15) : 'not available'}.',
              style: Theme.of(context)
                  .primaryTextTheme
                  .subhead
                  .copyWith(color: Colors.black),
            ),
          ],
        ),
      ),
    );
  }
}
