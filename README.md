# Celerity
Celerity - a game / visualization engine for Windows, written in C++, currently in it's third-ish iteration

Broken into two parts, a generic host application and a scene editor, with full support for developer-supplied content and entity component plug-ins, Celerity is an able platform to build visualizations, games, and simulations. The rendering system is currently fitted for deferred lighting, PBR materials, and efficiently manages states.

To start developing, load the main Celerity solution in Visual Studio and build the "Bootstrap (Debug)" target. Be prepared: it will be building the OpenGL wrapper code, which involves downloading and parsing header files and documentation (yes, the resulting code includes comments detailing the version of OpenGL the function comes from and what it does). Even if you leave it in the Bootstrap configuration, a CRC is generated based on the .h files and the parameters used to create it which, together, prevent re-processing of all the OpenGL headers. Even so, the initial download is slow enough that you'll want to switch to a Debug / Release config.

Pull requests are welcome.

<br>
Images:
<br>
<img src="https://github.com/keelanstuart/Celerity/blob/master/Media/screen_20230128.png" width="300" height="225" border="50"/>
<img src="https://github.com/keelanstuart/Celerity/blob/master/Media/screen_ed_20240209.png" width="300" height="225" border="50"/>

<br>
Video Links:
<br>
<a href="http://www.youtube.com/watch?feature=player_embedded&v=K8VkbBgpvhg" target="_blank">
<img src="http://img.youtube.com/vi/K8VkbBgpvhg/0.jpg" alt="2025 Demos" width="600" height="450" border="50" />
</a>

<a href="http://www.youtube.com/watch?feature=player_embedded&v=CBt5-uMS4ak" target="_blank">
<img src="http://img.youtube.com/vi/CBt5-uMS4ak/0.jpg" alt="Warehouse Flythrough" width="300" height="225" border="50" />
</a>

<a href="http://www.youtube.com/watch?feature=player_embedded&v=Wi8DE7UJTM0" target="_blank">
<img src="http://img.youtube.com/vi/Wi8DE7UJTM0/0.jpg" alt="Full hierarchies" width="300" height="225" border="50" />
</a>

<a href="http://www.youtube.com/watch?feature=player_embedded&v=QzydaeRiJyw" target="_blank">
<img src="http://img.youtube.com/vi/QzydaeRiJyw/0.jpg" alt="Simple Animation" width="300" height="225" border="50" />
</a>

<a href="http://www.youtube.com/watch?feature=player_embedded&v=UDmb0Wn8nBE" target="_blank">
<img src="http://img.youtube.com/vi/UDmb0Wn8nBE/0.jpg" alt="Floating Fountain" width="300" height="225" border="50" />
</a>

<a href="http://www.youtube.com/watch?feature=player_embedded&v=v6H-AJ6Iizc" target="_blank">
<img src="http://img.youtube.com/vi/v6H-AJ6Iizc/0.jpg" alt="Turbo Teaser 1" width="300" height="225" border="50" />
</a>

<a href="http://www.youtube.com/watch?feature=player_embedded&v=gavOlOR2wtU" target="_blank">
<img src="http://img.youtube.com/vi/gavOlOR2wtU/0.jpg" alt="Turbo Teaser 2" width="300" height="225" border="50" />
</a>

<br>
______________________________________________

To see some of the things Celerity has been used to create, check out this promotional video from my old company, Inventoractive: https://www.youtube.com/watch?v=mlXiiIEqRsw
