# JavaScript Functions in Celerity

This document provides an overview of JavaScript functions available in Celerity.

---

## Object Management

### `GetNumChildren(hrootobj)`
**Description:** Returns the number of children of the given object.  
**Parameters:**  
- `hrootobj` (handle): Object handle.  
**Returns:** Number of children.

### `GetChild(hrootobj, idx)`
**Description:** Retrieves a child object by index.  
**Parameters:**  
- `hrootobj` (handle): Object handle.  
- `idx` (number): Index of the child object.  
**Returns:** Child object handle.

### `FindObjByGUID(hrootobj, guid, recursive)`
**Description:** Finds an object by GUID.  
**Parameters:**  
- `hrootobj` (handle): Object handle.  
- `guid` (string): GUID of the object.  
- `recursive` (boolean): Whether to search recursively.  
**Returns:** Object handle.

### `FindFirstObjByName(hrootobj, name, recursive)`
**Description:** Finds the first object with the given name.  
**Parameters:**  
- `hrootobj` (handle): Object handle.  
- `name` (string): Object name.  
- `recursive` (boolean): Whether to search recursively.  
**Returns:** Object handle.

### `CreateObject(protoname, hparentobj)`
**Description:** Creates an object based on a prototype.  
**Parameters:**  
- `protoname` (string): Name of the prototype.  
- `hparentobj` (handle): Parent object handle.  
**Returns:** Created object handle.

### `DeleteObject(hobj)`
**Description:** Deletes the specified object.  
**Parameters:**  
- `hobj` (handle): Object handle.

### `GetParent(hobj)`
**Description:** Gets the parent of the specified object.  
**Parameters:**  
- `hobj` (handle): Object handle.  
**Returns:** Parent object handle.

### `SetParent(hobj, hnewparentobj)`
**Description:** Sets the parent of an object.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `hnewparentobj` (handle): New parent object handle.

### `SetObjectName(hobj, newname)`
**Description:** Renames an object.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `newname` (string): New name.

---

## Properties

### `FindProperty(hobj, name)`
**Description:** Finds a property by name on an object.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `name` (string): Property name.  
**Returns:** Property handle.

### `GetPropertyValue(hprop)`
**Description:** Retrieves the value of a property.  
**Parameters:**  
- `hprop` (handle): Property handle.  
**Returns:** Property value.

### `SetPropertyValue(hprop, val)`
**Description:** Sets the value of a property.  
**Parameters:**  
- `hprop` (handle): Property handle.  
- `val` (any): New value.

---

## Logging

### `Log(text)`
**Description:** Logs a message to the system log.  
**Parameters:**  
- `text` (string): Message to log.

---

## Prototypes & Objects

### `LoadPrototypes(filename)`
**Description:** Loads prototypes from a file.  
**Parameters:**  
- `filename` (string): Prototype file name.

### `LoadObject(hobj, filename)`
**Description:** Loads object data from a file.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `filename` (string): File name.

---

## Global Object Registry

### **Designations:**
- "worldroot"
- "skyboxroot"
- "camera.root"
- "camera.arm"
- "camera"
- "gui.root"
- "gui.camera"
- "player"

### `GetRegisteredObject(designation)`
**Description:** Retrieves a registered global object.  
**Parameters:**  
- `designation` (string): Object designation.  
**Returns:** Registered object handle.

### `RegisterObject(designation, hobj)`
**Description:** Registers an object globally.  
**Parameters:**  
- `designation` (string): Object designation.  
- `hobj` (handle): Object handle.

---

## Object Flags

### `IsObjFlagSet(hobj, flagname)`
**Description:** Checks if a flag is set on an object.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `flagname` (string): Flag name.  
**Returns:** `true` if set, otherwise `false`.

### `SetObjFlag(hobj, flagname, val)`
**Description:** Sets or clears a flag on an object.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `flagname` (string): Flag name.  
- `val` (boolean): `true` to set, `false` to clear.

---

## Quaternion & Euler Conversions

### `AdjustQuat(quat, axis, angle)`
**Description:** Adjusts a quaternion by rotating it around an axis.  
**Parameters:**  
- `quat` (object): Quaternion object.  
- `axis` (object): Rotation axis `{x, y, z}`.  
- `angle` (number): Rotation angle in radians.  
**Returns:** Adjusted quaternion.

### `EulerToQuat(euler)`
**Description:** Converts Euler angles to a quaternion.  
**Parameters:**  
- `euler` (object): Euler angles `{x, y, z}`.  
**Returns:** Quaternion `{x, y, z, w}`.

### `QuatToEuler(quat)`
**Description:** Converts a quaternion to Euler angles.  
**Parameters:**  
- `quat` (object): Quaternion `{x, y, z, w}`.  
**Returns:** Euler angles `{x, y, z}`.

---

## Sound

### `PlaySound(filename, volmod, pitchmod, loop, pos)`
**Description:** Plays a sound.  
**Parameters:**  
- `filename` (string): Sound file.  
- `volmod` (number): Volume modifier.  
- `pitchmod` (number): Pitch modifier.  
- `loop` (boolean): Whether to loop.  
- `pos` (object): Position `{x, y, z}`.

---

## Model Manipulation

### `GetModelNodeIndex(hobj, nodename)`
**Description:** Gets the index of a model node.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `nodename` (string): Node name.  
**Returns:** Node index.

### `GetModelNodeCount(hobj)`
**Description:** Returns the number of nodes in a model.  
**Parameters:**  
- `hobj` (handle): Object handle.  
**Returns:** Number of nodes.

### `GetModelInstNodePos(hobj, nodeidx)`
**Description:** Gets the position of a model instance node.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `nodeidx` (number): Node index.  
**Returns:** Position `{x, y, z}`.

### `GetModelInstNodeOri(hobj, nodeidx)`
**Description:** Gets the orientation of a model instance node.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `nodeidx` (number): Node index.  
**Returns:** Orientation `{x, y, z, w}`.

### `SetModelInstNodePos(hobj, nodeidx, pos)`
**Description:** Sets the position of a model instance node.  
**Parameters:**  
- `hobj` (handle): Object handle.  
- `nodeidx` (number): Node index.  
- `pos` (object): Position `{x, y, z}`.

---

## Collision Detection

### `CreateCollisionResults()`
**Description:** Creates a structure for storing collision results.  
**Returns:** Collision results object.

### `CheckCollisions(hrootobj, raypos, raydir, collinfo)`
**Description:** Checks for collisions along a ray.  
**Parameters:**  
- `hrootobj` (handle): Root object.  
- `raypos` (object): Ray origin `{x, y, z}`.  
- `raydir` (object): Ray direction `{x, y, z}`.  
- `collinfo` (object): Collision results.

---

## Color Packing & Unpacking

### `PackColorFromIntVec(coloriv)`
**Description:** Packs an integer color vector into an integer.  
**Parameters:**  
- `coloriv` (object): Color `{r, g, b, a}`.  
**Returns:** Packed integer color.

### `UnpackColorToFloatVec(color)`
**Description:** Unpacks a packed integer color into a float vector.  
**Parameters:**  
- `color` (integer): Packed color.  
**Returns:** `{r, g, b, a}` with float values in `[0,1]`.

---

# TinyJS Math and Vector API Documentation

## Math Functions

### `Math.abs(a)`
Returns the absolute value of `a`.
- **Parameters**: `a` (number)
- **Returns**: Absolute value of `a`

### `Math.round(a)`
Rounds `a` to the nearest integer.
- **Parameters**: `a` (number)
- **Returns**: Rounded value of `a`

### `Math.min(a, b)`
Returns the smaller of two numbers.
- **Parameters**: `a` (number), `b` (number)
- **Returns**: Minimum of `a` and `b`

### `Math.max(a, b)`
Returns the larger of two numbers.
- **Parameters**: `a` (number), `b` (number)
- **Returns**: Maximum of `a` and `b`

### `Math.range(x, a, b)`
Clamps `x` between `a` and `b`.
- **Parameters**: `x` (number), `a` (number), `b` (number)
- **Returns**: `x` constrained to `[a, b]`

### `Math.sign(a)`
Returns the sign of `a` (-1 for negative, 0 for zero, 1 for positive).
- **Parameters**: `a` (number)
- **Returns**: Sign of `a`

### `Math.even(a)`
Checks if `a` is even.
- **Parameters**: `a` (integer)
- **Returns**: `1` if even, `0` if odd

### `Math.random(lo, hi)`
Returns a random number between `lo` and `hi`.
- **Parameters**: `lo` (number), `hi` (number)
- **Returns**: Random value in `[lo, hi]`

### `Math.PI()`
Returns the value of π.
- **Returns**: `π`

### `Math.toDegrees(a)`
Converts radians to degrees.
- **Parameters**: `a` (number)
- **Returns**: Degrees equivalent of `a`

### `Math.toRadians(a)`
Converts degrees to radians.
- **Parameters**: `a` (number)
- **Returns**: Radians equivalent of `a`

### Trigonometric Functions

- `Math.sin(a)`: Returns the sine of `a` (radians).
- `Math.asin(a)`: Returns the arcsine of `a`.
- `Math.cos(a)`: Returns the cosine of `a`.
- `Math.acos(a)`: Returns the arccosine of `a`.
- `Math.tan(a)`: Returns the tangent of `a`.
- `Math.atan(a)`: Returns the arctangent of `a`.
- `Math.sinh(a)`: Returns the hyperbolic sine of `a`.
- `Math.asinh(a)`: Returns the hyperbolic arcsine of `a`.
- `Math.cosh(a)`: Returns the hyperbolic cosine of `a`.
- `Math.acosh(a)`: Returns the hyperbolic arccosine of `a`.
- `Math.tanh(a)`: Returns the hyperbolic tangent of `a`.
- `Math.atanh(a)`: Returns the hyperbolic arctangent of `a`.

### Logarithmic and Exponential Functions

- `Math.E()`: Returns Euler's constant `e`.
- `Math.log(a)`: Returns the natural logarithm of `a`.
- `Math.log10(a)`: Returns the base-10 logarithm of `a`.
- `Math.exp(a)`: Returns `e` raised to `a`.
- `Math.pow(a, b)`: Returns `a` raised to the power `b`.

### Square Functions

- `Math.sqr(a)`: Returns `a` squared.
- `Math.sqrt(a)`: Returns the square root of `a`.

### Interpolation Functions

- `Math.lerp(a, b, t)`: Linearly interpolates between `a` and `b` by `t`.
- `Math.slerp(a, b, t)`: Spherically interpolates between two quaternions.

## Vector Functions (`Vec3`)

### `Vec3.dot(a, b)`
Computes the dot product of two 3D vectors.
- **Parameters**: `a` (Vec3), `b` (Vec3)
- **Returns**: Dot product (number)

### `Vec3.cross(a, b)`
Computes the cross product of two 3D vectors.
- **Parameters**: `a` (Vec3), `b` (Vec3)
- **Returns**: Cross product (Vec3)

### `Vec3.length(a)`
Computes the length of a 3D vector.
- **Parameters**: `a` (Vec3)
- **Returns**: Length (number)

### `Vec3.normalize(a)`
Normalizes a 3D vector.
- **Parameters**: `a` (Vec3)
- **Returns**: Normalized vector (Vec3)

### Vector Arithmetic

- `Vec3.add(a, b)`: Adds two 3D vectors.
- `Vec3.sub(a, b)`: Subtracts `b` from `a`.
- `Vec3.mul(a, b)`: Multiplies `a` by `b` (element-wise).
- `Vec3.div(a, b)`: Divides `a` by `b` (element-wise).


## General Functions

### `exec(jsCode)`
Executes the given JavaScript code.
- **Parameters**: `jsCode` (string)
- **Returns**: None

### `trace()`
Prints a trace of all variables in the TinyJS environment.
- **Returns**: None

## Object Functions

### `Object.dump()`
Dumps the contents of the object to the console.
- **Returns**: None

### `Object.clone()`
Creates a shallow copy of the object.
- **Returns**: Cloned object

## Math Functions

### `Math.rand()`
Returns a random floating-point number between 0 and 1.
- **Returns**: Random float (0 ≤ value < 1)

### `Math.randInt(min, max)`
Returns a random integer between `min` and `max`.
- **Parameters**: `min` (integer), `max` (integer)
- **Returns**: Random integer in `[min, max]`

## String Functions

### `charToInt(ch)`
Converts a character to its integer ASCII value.
- **Parameters**: `ch` (string, single character)
- **Returns**: ASCII integer value

### `String.indexOf(search)`
Finds the position of `search` within the string.
- **Parameters**: `search` (string)
- **Returns**: Index position, or `-1` if not found

### `String.substring(lo, hi)`
Extracts a substring between indices `lo` and `hi`.
- **Parameters**: `lo` (integer), `hi` (integer)
- **Returns**: Extracted substring

### `String.charAt(pos)`
Returns the character at the specified position.
- **Parameters**: `pos` (integer)
- **Returns**: Character at position `pos`

### `String.charCodeAt(pos)`
Returns the ASCII value of the character at position `pos`.
- **Parameters**: `pos` (integer)
- **Returns**: ASCII integer value

### `String.fromCharCode(char)`
Creates a character from an ASCII code.
- **Parameters**: `char` (integer)
- **Returns**: Corresponding character

### `String.split(separator)`
Splits a string into an array using the given separator.
- **Parameters**: `separator` (string)
- **Returns**: Array of substrings

### `String.includes(str, sensitive)`
Checks if the string contains `str`.
- **Parameters**:
  - `str` (string) - The substring to search for
  - `sensitive` (boolean, optional) - Case sensitivity (default: `true`)
- **Returns**: `true` if found, `false` otherwise

## Integer Functions

### `Integer.parseInt(str)`
Parses `str` into an integer.
- **Parameters**: `str` (string)
- **Returns**: Parsed integer

### `Integer.valueOf(str)`
Returns the integer value of a single character.
- **Parameters**: `str` (string, single character)
- **Returns**: ASCII integer value

## Array Functions

### `Array.contains(obj)`
Checks if the array contains `obj`.
- **Parameters**: `obj` (any type)
- **Returns**: `true` if found, `false` otherwise

### `Array.remove(obj)`
Removes all occurrences of `obj` from the array.
- **Parameters**: `obj` (any type)
- **Returns**: None (modifies array in-place)

### `Array.join(separator)`
Joins array elements into a string using `separator`.
- **Parameters**: `separator` (string)
- **Returns**: Joined string

