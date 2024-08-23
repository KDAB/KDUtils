# KDUtils

KDUtils is a library which provides both helpers and wrappers around the C++ STL
and a base for cross-platform applications, similar to QtCore.

Also check out [KDBindings](https://github.com/KDAB/KDBindings) for an
implementation of properties, signals, and slots. Used heavily in KDUtils.

## KDFoundation

The KDFoundation namespace provides an event loop, an `EventReceiver` abstract
class, and an `Object` type which is an event receiver that can have a parent
`Object` and any number of children.

On top of that, you get:

- Event
- FileDescriptorNotifier
- Timer
- EventQueue: a thread-safe stack of events
- Postman: a way of passing events down a series of receivers
- CoreApplication: a class that connects the EventQueue, Postman, and event loop.

## KDGui

KDGui provides a cross-platform windowing solution, built on top of the
CoreApplication.

You get:

- Window
- Keyboard and mouse events
- GuiApplication

## KDUtils/KDUtils

This namespace is the namesake of the entire library, and contains the STL
wrappers and helpers.

You get:

- ByteArray
- Dir
- File
- ElapsedTimer
- Flags
- Url
- Logging factory helper

## Building

On Linux:

```bash
sudo apt install libxkbcommon-dev libxcb-xkb-dev libxkbcommon-x11-dev wayland-scanner++ wayland-protocols libwayland-dev
```

For debug builds with code coverage:

```bash
sudo apt install gcovr
```

## Contact

- Visit us on GitHub: <https://github.com/KDAB/KDUtils>
- Email <info@kdab.com> for questions about copyright, licensing or commercial support.

## Licensing

KDUtils is © Klarälvdalens Datakonsult AB and is available under the
terms of the [MIT](LICENSES/MIT.txt) license.

Contact KDAB at <info@kdab.com> to inquire about commercial licensing.

## Get Involved

Please submit your contributions or issue reports from our GitHub space at
<https://github.com/KDAB/KDUtils>.

Contact <info@kdab.com> for more information.

## About KDAB

KDUtils is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and
OpenGL applications across desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications
from scratch and in porting from all popular and legacy frameworks to Qt.
We continue to help develop parts of Qt and are one of the major contributors
to the Qt Project. We can give advanced or standard trainings anywhere
around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit <https://www.kdab.com> to meet the people who write code like this.

Stay up-to-date with KDAB product announcements:

- [KDAB Newsletter](https://news.kdab.com)
- [KDAB Blogs](https://www.kdab.com/category/blogs)
- [KDAB on Twitter](https://twitter.com/KDABQt)
