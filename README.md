# Celerity <i>- a modern C++ engine for visualization, simulation, and game development on Windows.</i>

Celerity is a flexible, modular engine built from the ground up in C++. Now in its third major iteration, it’s designed to serve as both a powerful development platform and a practical content creation environment. Whether you’re building a real-time visualization, a full game, or a simulation, Celerity offers the tools and extensibility to get you there.


### Key Features
- Modular & Generic Design:
Celerity is built as a truly generic engine. You can use as much or as little of the system as needed in your own applications via a clean, well-structured API.

- Extensible via Plug-ins:
Easily extend functionality by writing custom plug-ins... for example, to support proprietary file formats or add new runtime object behaviors.

- Embedded JavaScript:
Access object properties, handle input, play sounds, respond to events... all using JavaScript.

- Dual Architecture for "Out of the Box" Development & Content Creation:
The platform is split into two main parts:<br>
   - A generic host application, suitable for embedding or launching projects<br>
   - A fully-featured scene editor for creating and modifying game and simulation content

- Robust Rendering & Simulation Framework:
Built on OpenGL with a custom wrapper layer, Celerity includes tooling that generates rich, versioned wrapper code (with inline documentation) to simplify development and maximize compatibility.


### Getting Started
To start developing, download the latest SDK release and install it wherever you prefer. The environment variable CELERITY_SDK will be created or updated to reflect the root install path automatically.

All the sub-systems of Celerity are made available through the c3::System interface. Create one like this:

```C++
#include <C3.h>

void your_code()
{
  c3::System *psys = c3::System::Create();

  // access all Celerity systems through psys

  psys->Release();
}
```


### Building the Engine
To build Celerity from source:
- Open the Celerity.sln solution file in Visual Studio.

- Build the Bootstrap (Debug) target.

- The initial build generates the OpenGL wrapper code, which will automatically download OpenGL headers, parse them, and adds comments for all functions. A CRC is calculated from the headers and configuration options, preventing unnecessary regeneration on subsequent builds. Note: the initial bootstrap process can be slow due to downloads, so switching to Debug or Release afterward is recommended.


### Contributions
Pull requests are welcome! Contributions that improve flexibility, compatibility, performance, or ease of use are encouraged.

<br>
Video Links:
<br>
<a href="https://www.youtube.com/playlist?list=PLaed72lE3UjhVFmb2GQAxGLLvyB35kld-&feature=player_embedded" target="_blank">
<img src="http://img.youtube.com/vi/K8VkbBgpvhg/0.jpg" alt="2026 Demos" width="600" height="450" border="50" />
</a>
<br>
______________________________________________
<br>

To see some of the things Celerity has been used to create, check out this promotional video from my old company, Inventoractive: https://www.youtube.com/watch?v=mlXiiIEqRsw
