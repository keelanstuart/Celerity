# Celerity

**A native C++ game, graphics, and simulation engine for Windows.**

Celerity is a complete development environment for building games, real-time visualizations, simulations, and other interactive applications.

Long-running and large in scope, Celerity encompasses everything from rendering, physics, audio, scripting, terrain, input, animation, resources, and application infrastructure to the tools used to put it all together. The engine has evolved through three major generations over more than two decades of development.

<table>
  <tr>
    <td width="50%" align="center" valign="top">
      <a href="https://www.youtube.com/playlist?list=PLaed72lE3UjhVFmb2GQAxGLLvyB35kld-&feature=player_embedded">
        <img src="https://img.youtube.com/vi/K8VkbBgpvhg/0.jpg" alt="2026 Demos" width="100%" />
      </a>
      <br>
      <em>2026 Celerity Demo Reel</em>
    </td>
    <td width="50%" align="center" valign="top">
      <a href="https://www.youtube.com/watch?v=mlXiiIEqRsw&feature=player_embedded">
        <img src="https://img.youtube.com/vi/mlXiiIEqRsw/0.jpg" alt="Inventoractive (my old company) Promotional Video" width="100%" />
      </a>
      <br>
      <em>Inventoractive's Promotional Video (my old company)</em>
    </td>
  </tr>
</table>

**The goal is always the same: make difficult things easier.**

Celerity gives you several levels at which to work. Build scenes and content visually in the editor... add behavior using embedded JavaScript... extend the engine with native plug-ins... or, use Celerity's individual systems directly from C++ when you need complete control.

Use as much or as little of it as you need.

## More Than a Rendering Engine

Celerity is designed as an integrated development platform rather than a collection of unrelated graphics libraries.

Its systems share a common object and property model, allowing the editor, scripting environment, serialization, plug-ins, and runtime code to operate on the same application data. Content created in the editor is the same content consumed by the runtime.

At the center is a component-based object model: objects acquire capabilities such as rendering, physics, scripting, audio, animation, or application-specific behavior through the components attached to them.

The result is an engine that can provide a productive out-of-the-box environment without putting a wall between you and the underlying systems.

## What Celerity Includes

* **Modern real-time rendering**: OpenGL-based rendering with programmable materials, deferred lighting, physically based shading, terrain, model rendering, and the infrastructure needed to build complete 3D scenes.

* **Physics and simulation**: Integrated rigid-body physics and collision support designed to work naturally with Celerity's scene objects and component system.

* **Embedded scripting**: JavaScript-like scripting for object behavior, input handling, events, audio, properties, and gameplay or simulation logic without requiring a native rebuild.

* **Visual scene editing**: Create, inspect, configure, and assemble application content using Celerity's editor rather than constructing everything programmatically.

* **Component-based objects**: Add capabilities to objects through reusable components for rendering, physics, scripting, and other behaviors.

* **Properties everywhere**: A common property system allows data to be exposed consistently to native code, scripts, editors, serialization, and application-specific tooling.

* **Terrain / Procedural Geometry**: Integrated terrain rendering and interaction suitable for large visualization, simulation, and game environments.

* **Audio**: Runtime sound and audio support available from both native code and scripts.

* **Input and actions**: Abstract application input into named actions rather than coupling behavior directly to individual keys or devices.

* **Extensible plug-in architecture**: Add proprietary formats, specialized object behaviors, application-specific systems, or entirely new capabilities without modifying the core engine.

* **Resource and content management**: Common infrastructure for loading, locating, managing, and consuming application assets.

* **Native C++ API**: The same underlying systems remain directly available when higher-level tools and scripting aren't enough.

## Work at the Level You Need

One of Celerity's central design goals is to avoid forcing every problem through the same abstraction.

For content creation, use the editor.

For behavior and rapid iteration, use scripting.

For reusable or performance-sensitive functionality, write a native component or plug-in.

For specialized applications, use Celerity as an engine underneath your own application and access its subsystems directly.

The higher-level tools are there to remove work, not remove control.
