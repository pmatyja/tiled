#include "gidmapper.h"
#include "isometricrenderer.h"
#include "isometricsurface.h"
#include "map.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

#include <QtTest/QtTest>

#include <memory>

using namespace Tiled;

class test_IsometricRenderer : public QObject
{
    Q_OBJECT

private slots:
    void unconfiguredTileIsUnchanged();
    void projectedTilesetDefaultsToFlat();
    void projectedTilesetAllowsNormalTile();
    void rendersFlatFace();
    void rendersLeftFarFace();
    void rendersRightFarFace();
    void rendersLeftCloseFace();
    void rendersRightCloseFace();
    void overridesTileFacePerCell();
    void ignoresOverrideForUnconfiguredTile();
    void encodesSurfaceOverrideInCellFlags();
    void preservesSurfaceOverrideInGid();
    void appliesWorldOffset();
    void appliesNegativeWorldOffset();
    void appliesCellFlip();
    void reservesDrawMargins();

private:
    struct TestMap {
        std::unique_ptr<Map> map;
        SharedTileset tileset;
        Tile *tile;
        TileLayer *layer;
    };

    static TestMap createMap(const QString &face, bool projectedTileset = false);
    static QImage render(const TestMap &testMap);
    static QRect alphaBounds(const QImage &image);
};

test_IsometricRenderer::TestMap test_IsometricRenderer::createMap(const QString &face,
                                                                  bool projectedTileset)
{
    Map::Parameters parameters;
    parameters.orientation = Map::Isometric;
    parameters.width = 1;
    parameters.height = 1;
    parameters.tileWidth = 64;
    parameters.tileHeight = 32;

    auto map = std::make_unique<Map>(parameters);

    SharedTileset tileset = Tileset::create(QStringLiteral("textures"), 8, 8);
    if (projectedTileset)
        tileset->setProperty(QStringLiteral("isometricProjected"), true);
    QPixmap texture(8, 8);
    texture.fill(Qt::red);
    {
        QPainter painter(&texture);
        painter.fillRect(4, 0, 4, 8, Qt::blue);
    }
    Tile *tile = tileset->addTile(texture);
    if (!face.isEmpty())
        tile->setProperty(QStringLiteral("isometricFace"), face);

    auto layer = new TileLayer(QStringLiteral("surfaces"), 0, 0, 1, 1);
    layer->setCell(0, 0, Cell(tile));
    map->addTileset(tileset);
    map->addLayer(layer);

    return { std::move(map), tileset, tile, layer };
}

QImage test_IsometricRenderer::render(const TestMap &testMap)
{
    QImage image(160, 144, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.translate(32, 48);
    IsometricRenderer(testMap.map.get()).drawTileLayer(&painter, testMap.layer);
    return image;
}

QRect test_IsometricRenderer::alphaBounds(const QImage &image)
{
    QRect bounds;
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(image.pixel(x, y)) != 0)
                bounds |= QRect(x, y, 1, 1);
        }
    }
    return bounds;
}

void test_IsometricRenderer::unconfiguredTileIsUnchanged()
{
    const TestMap testMap = createMap(QString());
    QCOMPARE(alphaBounds(render(testMap)), QRect(32, 72, 8, 8));
}

void test_IsometricRenderer::projectedTilesetDefaultsToFlat()
{
    const TestMap testMap = createMap(QString(), true);
    QCOMPARE(isometricSurfaceForTile(testMap.tile).face, IsometricSurface::Flat);
    QCOMPARE(alphaBounds(render(testMap)), QRect(33, 48, 62, 32));
}

void test_IsometricRenderer::projectedTilesetAllowsNormalTile()
{
    TestMap testMap = createMap(QString(), true);
    testMap.tile->setProperty(QStringLiteral("isometricFace"), QStringLiteral("none"));

    QCOMPARE(isometricSurfaceForTile(testMap.tile).face, IsometricSurface::None);
    QCOMPARE(alphaBounds(render(testMap)), QRect(32, 72, 8, 8));
}

void test_IsometricRenderer::rendersFlatFace()
{
    const TestMap testMap = createMap(QStringLiteral("flat"));
    const QImage image = render(testMap);

    QVERIFY(qAlpha(image.pixel(64, 64)) != 0);
    QVERIFY(qAlpha(image.pixel(34, 49)) == 0);
    QCOMPARE(alphaBounds(image), QRect(33, 48, 62, 32));
}

void test_IsometricRenderer::rendersLeftFarFace()
{
    const TestMap testMap = createMap(QStringLiteral("leftFar"));
    QCOMPARE(alphaBounds(render(testMap)), QRect(32, 16, 32, 48));
}

void test_IsometricRenderer::rendersRightFarFace()
{
    const TestMap testMap = createMap(QStringLiteral("rightFar"));
    QCOMPARE(alphaBounds(render(testMap)), QRect(64, 16, 32, 48));
}

void test_IsometricRenderer::rendersLeftCloseFace()
{
    const TestMap testMap = createMap(QStringLiteral("leftClose"));
    QCOMPARE(alphaBounds(render(testMap)), QRect(64, 32, 32, 48));
}

void test_IsometricRenderer::rendersRightCloseFace()
{
    const TestMap testMap = createMap(QStringLiteral("rightClose"));
    QCOMPARE(alphaBounds(render(testMap)), QRect(32, 32, 32, 48));
}

void test_IsometricRenderer::overridesTileFacePerCell()
{
    TestMap testMap = createMap(QStringLiteral("flat"));
    Cell cell = testMap.layer->cellAt(0, 0);

    setIsometricSurfaceOverride(cell, IsometricSurface::LeftFar);
    testMap.layer->setCell(0, 0, cell);
    QCOMPARE(alphaBounds(render(testMap)), QRect(32, 16, 32, 48));

    setIsometricSurfaceOverride(cell, IsometricSurface::RightFar);
    testMap.layer->setCell(0, 0, cell);
    QCOMPARE(alphaBounds(render(testMap)), QRect(64, 16, 32, 48));

    setIsometricSurfaceOverride(cell, IsometricSurface::None);
    testMap.layer->setCell(0, 0, cell);
    QCOMPARE(alphaBounds(render(testMap)), QRect(33, 48, 62, 32));
}

void test_IsometricRenderer::ignoresOverrideForUnconfiguredTile()
{
    TestMap testMap = createMap(QString());
    Cell cell = testMap.layer->cellAt(0, 0);
    setIsometricSurfaceOverride(cell, IsometricSurface::LeftFar);
    testMap.layer->setCell(0, 0, cell);

    QCOMPARE(isometricSurfaceForCell(cell).face, IsometricSurface::None);
    QCOMPARE(alphaBounds(render(testMap)), QRect(32, 72, 8, 8));
}

void test_IsometricRenderer::encodesSurfaceOverrideInCellFlags()
{
    Cell cell;

    setIsometricSurfaceOverride(cell, IsometricSurface::LeftFar);
    QCOMPARE(isometricSurfaceOverride(cell), IsometricSurface::LeftFar);
    QVERIFY(cell.flippedAntiDiagonally());
    QVERIFY(!cell.rotatedHexagonal120());

    setIsometricSurfaceOverride(cell, IsometricSurface::RightFar);
    QCOMPARE(isometricSurfaceOverride(cell), IsometricSurface::RightFar);
    QVERIFY(!cell.flippedAntiDiagonally());
    QVERIFY(cell.rotatedHexagonal120());

    setIsometricSurfaceOverride(cell, IsometricSurface::Flat);
    QCOMPARE(isometricSurfaceOverride(cell), IsometricSurface::Flat);
    QVERIFY(cell.flippedAntiDiagonally());
    QVERIFY(cell.rotatedHexagonal120());

    setIsometricSurfaceOverride(cell, IsometricSurface::None);
    QCOMPARE(isometricSurfaceOverride(cell), IsometricSurface::None);
    QVERIFY(!cell.flippedAntiDiagonally());
    QVERIFY(!cell.rotatedHexagonal120());
}

void test_IsometricRenderer::preservesSurfaceOverrideInGid()
{
    const TestMap testMap = createMap(QStringLiteral("flat"));
    const GidMapper gidMapper(testMap.map->tilesets());
    Cell cell(testMap.tile);
    setIsometricSurfaceOverride(cell, IsometricSurface::RightFar);

    bool ok = false;
    const Cell decoded = gidMapper.gidToCell(gidMapper.cellToGid(cell), ok);

    QVERIFY(ok);
    QCOMPARE(decoded.tile(), testMap.tile);
    QCOMPARE(isometricSurfaceOverride(decoded), IsometricSurface::RightFar);
}

void test_IsometricRenderer::appliesWorldOffset()
{
    TestMap testMap = createMap(QStringLiteral("flat"));
    const QRect originalBounds = alphaBounds(render(testMap));

    testMap.tile->setProperty(QStringLiteral("isometricX"), 1.0);
    testMap.tile->setProperty(QStringLiteral("isometricZ"), 1.0);

    QCOMPARE(alphaBounds(render(testMap)), originalBounds.translated(32, -16));
}

void test_IsometricRenderer::appliesNegativeWorldOffset()
{
    TestMap testMap = createMap(QStringLiteral("flat"));
    const QRect originalBounds = alphaBounds(render(testMap));

    testMap.tile->setProperty(QStringLiteral("isometricX"), -1.0);
    testMap.tile->setProperty(QStringLiteral("isometricZ"), -1.0);

    QCOMPARE(alphaBounds(render(testMap)), originalBounds.translated(-32, 16));
}

void test_IsometricRenderer::appliesCellFlip()
{
    TestMap testMap = createMap(QStringLiteral("flat"));
    QCOMPARE(render(testMap).pixelColor(56, 60), QColor(Qt::red));

    Cell cell(testMap.tile);
    cell.setFlippedHorizontally(true);
    testMap.layer->setCell(0, 0, cell);

    QCOMPARE(render(testMap).pixelColor(56, 60), QColor(Qt::blue));
}

void test_IsometricRenderer::reservesDrawMargins()
{
    const TestMap testMap = createMap(QStringLiteral("flat"));

    QCOMPARE(isometricSurfaceDrawMargins(testMap.map->tileSize()),
             QMargins(32, 64, 32, 32));
    QCOMPARE(testMap.map->drawMargins(), QMargins(32, 64, 32, 32));
}

QTEST_MAIN(test_IsometricRenderer)
#include "test_isometricrenderer.moc"
