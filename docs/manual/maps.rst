Editing Maps
============

A map is a top-level asset file that contains :doc:`layers <layers>` and references :doc:`tileset
files <editing-tilesets>` (which can optionally be embedded). It defines the coordinate system, tile
size, orientation, render order and other map-wide configuration options that affect how the content
is displayed and exported.

Creating a Map
--------------

You can create a new map via *File -> New -> New Map…*. The New Map dialog
lets you set:

- **Orientation** (see :ref:`map-orientations`)
- **Tile Layer Format** (the storage format for tile data)
- **Render Order** (only for orthogonal and oblique maps)
- **Map Size** (fixed width/height or infinite)
- **Tile Size** (tile width/height in pixels)

All of these options can be changed later in the **Properties** panel, via
*Map -> Map Properties…*.

.. _map-orientations:

Map Orientations
----------------

The map orientation controls how the tile grid is projected and how tiles are
positioned relative to each other. Tiled supports the following orientations.

Orthogonal
~~~~~~~~~~

The classic top-down grid where rectangular tiles are arranged in straight rows
and columns. This is the most straightforward orientation.

Isometric
~~~~~~~~~

Isometric maps are projected to give a 3D-like appearance. Tiles are still arranged in a grid, but
are drawn as diamonds (by default Tiled doesn't transform your art, so you'll need to provide
diamond-shaped tiles). Object positions are stored in a projected coordinate space (see
:ref:`object-layer-introduction`).

Projecting Rectangular Surface Textures
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Tiled can project rectangular tile textures onto the three visible isometric
planes. This behavior is available automatically on isometric maps. In the
tileset Properties view, set ``Tile Projection`` to ``Isometric Projected`` to
make every tile in that tileset project as a flat surface by default. Tilesets
using ``Normal`` projection continue to render normally.

Selecting a tile shows an ``Isometric Surface`` group in the Properties view.
Its ``Default Face`` can be ``Normal``, ``Flat``, ``Left Far``, ``Right Far``,
``Left Close`` or ``Right Close``, and its X, Y and Z offsets range from -1
through 1. The offsets are signed displacements from the surface's default
world-tile anchor and default to 0.

The ``flat`` plane spans the positive X and Y axes. The ``left`` plane spans
the positive Y axis and extends down the Z axis, while the ``right`` plane spans
the positive X axis and extends down the Z axis. Tile flipping is applied to
the source texture before projection. The flat face is centered on the world
tile. For side faces, the bottom-center of the source texture is anchored to
the selected tile edge. The ``Far`` options use the upper tile edge and the
``Close`` options use the lower tile edge.

When using the Stamp Brush on an isometric map, the Tool Options toolbar shows
``Default``, ``Left``, ``Right`` and ``Flat`` buttons. ``Default`` uses each
tile's effective tile or tileset default, including its far or close placement.
The other buttons override the plane for newly painted cells, while keeping the
tile's XYZ offset. ``Left`` and ``Right`` overrides use the far edge. These
overrides are saved with the cell. Horizontal and vertical flipping still mirror
the source texture, but diagonal and hexagonal rotation are reserved for storing
the surface override on projectable tiles.

Surface tiles follow the tile layer's existing drawing order; this feature does
not add depth sorting between overlapping cells. Projection applies to tile
layers in isometric maps, not to tile objects.

Isometric (Staggered)
~~~~~~~~~~~~~~~~~~~~~

Staggered isometric maps also use diamond-shaped tiles, but the grid is
staggered every other row or column. The exact staggering is controlled by the
**Stagger Axis** and **Stagger Index** map properties.

This orientation allows a map based on isometric tiles to still have an overall rectangular shape.

Hexagonal (Staggered)
~~~~~~~~~~~~~~~~~~~~~

Hexagonal maps use hex tiles arranged in a staggered grid. Like staggered
isometric maps, the staggering is controlled by **Stagger Axis** and
**Stagger Index**. Hexagonal maps also support a **Hex Side Length** that
defines the length of the straight edges of the hex tile.

The following table shows how **Stagger Axis** relates to the two common hex
tile orientations:

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Stagger Axis
     - Hex Tile Orientation
   * - X
     - Pointy-top
   * - Y
     - Straight-top


.. raw:: html

    <div class="new">New in Tiled 1.12</div>

Oblique
~~~~~~~

Oblique maps apply a skew transform to achieve a pseudo-3D projection. The amount of skew is
controlled by the **Skew** map property, in pixels.

**Skew X** determines the horizontal offset applied for each row, while **Skew Y** determines the
vertical offset applied for each column.

Map Properties
--------------

The following properties are especially relevant when configuring a map.

Map Size (Fixed vs. Infinite)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Maps can be **fixed size** or **infinite**. A fixed map has a set width and
height in tiles, while an infinite map has an auto-growing canvas that expands
as you paint. The choice impacts how tile layers are stored.

For details on working with infinite maps and converting between the two, see
:doc:`using-infinite-maps`.

Tile Size
~~~~~~~~~

The tile width and height define the size of each tile in pixels. These values affect the tile grid
and the map bounds, but does not affect the size at which tiles are rendered (unless the relevant
tileset has **Tile Render Size** set to **Map Grid Size**).

Parallax Origin
~~~~~~~~~~~~~~~

The parallax origin defines the reference point for :ref:`parallax scrolling
factor <parallax-factor>` on layers. It is stored per map and defaults to
(0, 0), which is the top-left of the map's bounding box.

Tile Layer Format
~~~~~~~~~~~~~~~~~

The tile layer format determines how tile data is stored when saving or
exporting the map. Some formats are more compact or faster to parse than
others. You can change the format at any time in the **Properties** panel,
via *Map -> Map Properties…*.
