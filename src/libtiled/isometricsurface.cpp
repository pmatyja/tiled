/*
 * isometricsurface.cpp
 * Copyright 2026, The Tiled Authors
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "isometricsurface.h"

#include "map.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QtMath>

using namespace Tiled;

static const QString isometricProjectedProperty = QStringLiteral("isometricProjected");
static const QString isometricFaceProperty = QStringLiteral("isometricFace");
static const QString isometricXProperty = QStringLiteral("isometricX");
static const QString isometricYProperty = QStringLiteral("isometricY");
static const QString isometricZProperty = QStringLiteral("isometricZ");

static qreal relativeCoordinate(const QVariant &value)
{
    if (!value.isValid())
        return 0;

    return qBound(qreal(-1), value.toReal(), qreal(1));
}

static IsometricSurface::Face faceFromString(const QString &value)
{
    const QString face = value.toLower();
    if (face == QLatin1String("flat"))
        return IsometricSurface::Flat;
    if (face == QLatin1String("leftfar")
            || face == QLatin1String("left far")
            || face == QLatin1String("left-far")
            || face == QLatin1String("left_far"))
        return IsometricSurface::LeftFar;
    if (face == QLatin1String("rightfar")
            || face == QLatin1String("right far")
            || face == QLatin1String("right-far")
            || face == QLatin1String("right_far"))
        return IsometricSurface::RightFar;
    if (face == QLatin1String("leftclose")
            || face == QLatin1String("left close")
            || face == QLatin1String("left-close")
            || face == QLatin1String("left_close")
            || face == QLatin1String("left"))
        return IsometricSurface::LeftClose;
    if (face == QLatin1String("rightclose")
            || face == QLatin1String("right close")
            || face == QLatin1String("right-close")
            || face == QLatin1String("right_close")
            || face == QLatin1String("right"))
        return IsometricSurface::RightClose;
    return IsometricSurface::None;
}

bool Tiled::isometricSurfaceRenderingEnabled(const Map *map)
{
    return map && map->orientation() == Map::Isometric;
}

IsometricSurface Tiled::isometricSurfaceForTile(const Tile *tile)
{
    IsometricSurface surface;
    if (!tile)
        return surface;

    surface.x = relativeCoordinate(tile->property(isometricXProperty));
    surface.y = relativeCoordinate(tile->property(isometricYProperty));
    surface.z = relativeCoordinate(tile->property(isometricZProperty));

    const QVariant faceValue = tile->property(isometricFaceProperty);
    if (!faceValue.isValid()) {
        if (tile->tileset()->property(isometricProjectedProperty).toBool())
            surface.face = IsometricSurface::Flat;
        return surface;
    }

    surface.face = faceFromString(faceValue.toString());
    if (surface.face == IsometricSurface::None)
        return surface;

    return surface;
}

IsometricSurface Tiled::isometricSurfaceForCell(const Cell &cell)
{
    IsometricSurface surface = isometricSurfaceForTile(cell.tile());
    if (surface.face == IsometricSurface::None)
        return surface;

    const IsometricSurface::Face override = isometricSurfaceOverride(cell);
    if (override != IsometricSurface::None)
        surface.face = override;

    return surface;
}

IsometricSurface::Face Tiled::isometricSurfaceOverride(const Cell &cell)
{
    const bool firstBit = cell.flippedAntiDiagonally();
    const bool secondBit = cell.rotatedHexagonal120();

    if (firstBit && secondBit)
        return IsometricSurface::Flat;
    if (firstBit)
        return IsometricSurface::LeftFar;
    if (secondBit)
        return IsometricSurface::RightFar;
    return IsometricSurface::None;
}

void Tiled::setIsometricSurfaceOverride(Cell &cell, IsometricSurface::Face face)
{
    cell.setFlippedAntiDiagonally(face == IsometricSurface::LeftFar
                                  || face == IsometricSurface::LeftClose
                                  || face == IsometricSurface::Flat);
    cell.setRotatedHexagonal120(face == IsometricSurface::RightFar
                               || face == IsometricSurface::RightClose
                               || face == IsometricSurface::Flat);
}

bool Tiled::isIsometricSurfaceProperty(const QString &name)
{
    return name == isometricProjectedProperty
            || name == isometricFaceProperty
            || name == isometricXProperty
            || name == isometricYProperty
            || name == isometricZProperty;
}

bool Tiled::tileLayerUsesIsometricSurfaces(const TileLayer *layer)
{
    if (!layer || !isometricSurfaceRenderingEnabled(layer->map()))
        return false;

    for (const Cell &cell : *layer) {
        if (isometricSurfaceForTile(cell.tile()).face != IsometricSurface::None)
            return true;
    }

    return false;
}

QMargins Tiled::isometricSurfaceDrawMargins(const QSize &tileSize)
{
    return QMargins(qCeil(tileSize.width() / 2.0),
                    qCeil(tileSize.height() * 2.0),
                    qCeil(tileSize.width() / 2.0),
                    tileSize.height());
}
