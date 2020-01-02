# Celerity
Celerity - a game / visualization engine for Windows, written in C++, currently in it's third-ish iteration

Project current requires the following support libraries to be cloned into ./third-party: <a href="https://github.com/keelanstuart/powerprops">PowerProps</a>, <a href="https://github.com/keelanstuart/mqme">mqme</a>, <a href="https://github.com/keelanstuart/Pool">Pool</a>, <a href="https://github.com/keelanstuart/genio">GenIO</a>, <a href="https://github.com/g-truc/glm">glm</a>, <a href="https://github.com/leethomason/tinyxml2">tinyxml2</a>, and <a href="https://github.com/keelanstuart/GLlibgen">GLlibgen</a>

Initially, you should build the "Bootstrap" target.

______________________________________________

Celerity has been around since roughly 2001, beginning life as a DirectX 8.1 engine... eventually moving to DirectX 9... and finally being ported to OpenGL. The initial reasons for using DirectX (better drivers, rapid adoption of new hardware features, and lots of helper library functions in D3DX) went away sometime in the last decade, or so, and have finally propmpted a re-write using OpenGL. Why not Vulkan or DX12? I think they (the hardware vendors and Microsoft) have tried to edge out smaller-scale studios by shifting the burden of software development away from themselves... and I'm not saying they're unwise to do this, but I believe OpenGL, at this point, provides a better platform for lone wolves.

No one should expect Celerity to become the next "big thing", but it was licensed to two companies in the past (in the DX8.1 form), and so therefore may have the pedigree to be a platform for small-scale PC-based graphics applications.

Stay tuned for rapid updates, since most of it will be ports from older code bases.
