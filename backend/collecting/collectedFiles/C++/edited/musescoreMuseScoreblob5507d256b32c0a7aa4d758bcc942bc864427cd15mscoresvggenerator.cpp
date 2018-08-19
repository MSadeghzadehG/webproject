

#include "svggenerator.h"
#include "libmscore/mscore.h"




static void translate_color(const QColor &color, QString *color_string,
                            QString *opacity_string)
{
    Q_ASSERT(color_string);
    Q_ASSERT(opacity_string);

    *color_string =
        QString::fromLatin1("#%1%2%3")
        .arg(color.red(), 2, 16, QLatin1Char('0'))
        .arg(color.green(), 2, 16, QLatin1Char('0'))
        .arg(color.blue(), 2, 16, QLatin1Char('0'));
    *opacity_string = QString::number(color.alphaF());
}

static void translate_dashPattern(QVector<qreal> pattern, const qreal& width, QString *pattern_string)
{
    Q_ASSERT(pattern_string);

        foreach (qreal entry, pattern)
        *pattern_string += QString::fromLatin1("%1,").arg(entry * width);

    pattern_string->chop(1);
}

static QString getClass(const Ms::Element *e)
{
    Ms::ElementType eType;
              QString eName;

        if (e == NULL)
        return eName; 
    eType = e->type();
    eName = e->name(eType);

    
    return eName;
}

class SvgPaintEnginePrivate
{
public:
    SvgPaintEnginePrivate()
    {
        size = QSize();
        viewBox = QRectF();
        outputDevice = 0;
        resolution = Ms::DPI;

        attributes.title = QLatin1String("MuseScore SVG Document");
        attributes.description = QString("Generated by MuseScore %1").arg(VERSION);
    }

    QSize size;
    QRectF viewBox;
    QIODevice *outputDevice;
    QTextStream *stream;
    int resolution;

    QString header;
    QString body;

    QBrush brush;
    QPen pen;
    QMatrix matrix;


    struct _attributes {
        QString title;
        QString description;
    } attributes;
};

static inline QPaintEngine::PaintEngineFeatures svgEngineFeatures()
{
    return QPaintEngine::PaintEngineFeatures(
           QPaintEngine::AllFeatures
        & ~QPaintEngine::PatternBrush
        & ~QPaintEngine::PerspectiveTransform
        & ~QPaintEngine::ConicalGradientFill
        & ~QPaintEngine::PorterDuff);
}

class SvgPaintEngine : public QPaintEngine
{
    friend class SvgGenerator; 
    Q_DECLARE_PRIVATE(SvgPaintEngine)

private:
    QString     stateString;
    QTextStream stateStream;
    SvgPaintEnginePrivate *d_ptr;

    qreal _dx;
    qreal _dy;

protected:
    const Ms::Element* _element = NULL;

#define SVG_SPACE    ' '
#define SVG_QUOTE    "\""
#define SVG_COMMA    ","
#define SVG_GT       ">"
#define SVG_PX       "px"
#define SVG_NONE     "none"
#define SVG_EVENODD  "evenodd"
#define SVG_BUTT     "butt"
#define SVG_SQUARE   "square"
#define SVG_ROUND    "round"
#define SVG_MITER    "miter"
#define SVG_BEVEL    "bevel"
#define SVG_ONE      "1"
#define SVG_BLACK    "#000000"

#define SVG_BEGIN    "<svg"
#define SVG_END      "</svg>"

#define SVG_WIDTH    " width=\""
#define SVG_HEIGHT   " height=\""
#define SVG_VIEW_BOX " viewBox=\""

#define SVG_X        " x="
#define SVG_Y        " y="

#define SVG_POINTS   " points=\""
#define SVG_D        " d=\""
#define SVG_MOVE     'M'
#define SVG_LINE     'L'
#define SVG_CURVE    'C'

#define SVG_CLASS    " class=\""

#define SVG_ELEMENT_END  "/>"
#define SVG_RPAREN_QUOTE ")\""

#define SVG_TITLE_BEGIN "<title>"
#define SVG_TITLE_END   "</title>"
#define SVG_DESC_BEGIN  "<desc>"
#define SVG_DESC_END    "</desc>"

#define SVG_IMAGE       "<image"
#define SVG_PATH        "<path"
#define SVG_POLYLINE    "<polyline"

#define SVG_PRESERVE_ASPECT " preserveAspectRatio=\""

#define SVG_FILL            " fill=\""
#define SVG_STROKE          " stroke=\""
#define SVG_STROKE_WIDTH    " stroke-width=\""
#define SVG_STROKE_LINECAP  " stroke-linecap=\""
#define SVG_STROKE_LINEJOIN " stroke-linejoin=\""
#define SVG_STROKE_DASHARRAY " stroke-dasharray=\""
#define SVG_STROKE_DASHOFFSET " stroke-dashoffset=\""
#define SVG_STROKE_MITERLIMIT " stroke-miterlimit=\""

#define SVG_OPACITY         " opacity=\""
#define SVG_FILL_OPACITY    " fill-opacity=\""
#define SVG_STROKE_OPACITY  " stroke-opacity=\""

#define SVG_FONT_FAMILY     " font-family=\""
#define SVG_FONT_SIZE       " font-size=\""

#define SVG_FILL_RULE       " fill-rule=\"evenodd\""
#define SVG_VECTOR_EFFECT   " vector-effect=\"non-scaling-stroke\""

#define SVG_MATRIX    " transform=\"matrix("

public:
    SvgPaintEngine()
        : QPaintEngine(svgEngineFeatures()),
          stateStream(&stateString)
    {
        d_ptr = new SvgPaintEnginePrivate;
    }

    bool begin(QPaintDevice *device);
    bool end();

    void updateState(const QPaintEngineState &state);
    void popGroup();

    void drawPath(const QPainterPath &path);
    void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    void drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                   Qt::ImageConversionFlag = Qt::AutoColor);

    QPaintEngine::Type type() const { return QPaintEngine::SVG; }

    QSize size() const { return d_func()->size; }
    void setSize(const QSize &size) {
        Q_ASSERT(!isActive());
        d_func()->size = size;
    }

    QRectF viewBox() const { return d_func()->viewBox; }
    void setViewBox(const QRectF &viewBox) {
        Q_ASSERT(!isActive());
        d_func()->viewBox = viewBox;
    }

    QString documentTitle() const { return d_func()->attributes.title; }
    void setDocumentTitle(const QString &title) {
        d_func()->attributes.title = title;
    }

    QString documentDescription() const { return d_func()->attributes.description; }
    void setDocumentDescription(const QString &description) {
        d_func()->attributes.description = description;
    }

    QIODevice *outputDevice() const { return d_func()->outputDevice; }
    void setOutputDevice(QIODevice *device) {
        Q_ASSERT(!isActive());
        d_func()->outputDevice = device;
    }

    int resolution() { return d_func()->resolution; }
    void setResolution(int resolution) {
        Q_ASSERT(!isActive());
        d_func()->resolution = resolution;
    }


    inline QTextStream &stream()
    {
        return *d_func()->stream;
    }

                const QString qpenToSvg(const QPen &spen)
    {
        QString     qs;
        QTextStream qts(&qs);

        QString color, colorOpacity;

                switch (spen.style()) {
        case Qt::NoPen:
            return qs;             break;

        case Qt::SolidLine:
        case Qt::DashLine:
        case Qt::DotLine:
        case Qt::DashDotLine:
        case Qt::DashDotDotLine:
        case Qt::CustomDashLine: {
                                    translate_color(spen.color(), &color, &colorOpacity);

                        qts << SVG_STROKE << color << SVG_QUOTE;

                        if (colorOpacity != SVG_ONE)
                qts << SVG_STROKE_OPACITY << colorOpacity << SVG_QUOTE;

                        if (spen.style() == Qt::SolidLine)
                break;

                        qreal penWidth = spen.width() == 0 ? qreal(1) : spen.widthF();

            QString dashPattern, dashOffset;
            translate_dashPattern(spen.dashPattern(), penWidth, &dashPattern);
            dashOffset = QString::number(spen.dashOffset() * penWidth); 
            qts << SVG_STROKE_DASHARRAY  << dashPattern << SVG_QUOTE;
            qts << SVG_STROKE_DASHOFFSET << dashOffset  << SVG_QUOTE;
            break; }
        default:
            qWarning("Unsupported pen style");
            break;
        }
                if (spen.widthF() > 0 && spen.widthF() != 1) {
                        qts.setRealNumberPrecision(2);             qts.setRealNumberNotation(QTextStream::FixedNotation);
            qts << SVG_STROKE_WIDTH << spen.widthF() << SVG_QUOTE;
            qts.setRealNumberNotation(QTextStream::SmartNotation);
        }
                switch (spen.capStyle()) {
        case Qt::FlatCap:
                                    break;
        case Qt::SquareCap:
            qts << SVG_STROKE_LINECAP << SVG_SQUARE << SVG_QUOTE;
            break;
        case Qt::RoundCap:
            qts << SVG_STROKE_LINECAP << SVG_ROUND << SVG_QUOTE;
            break;
        default:
            qWarning("Unhandled cap style");
            break;
        }
                switch (spen.joinStyle()) {
        case Qt::MiterJoin:
        case Qt::SvgMiterJoin:
            qts << SVG_STROKE_LINEJOIN   << SVG_MITER         << SVG_QUOTE
                << SVG_STROKE_MITERLIMIT << spen.miterLimit() << SVG_QUOTE;
            break;
        case Qt::BevelJoin:
            qts << SVG_STROKE_LINEJOIN   << SVG_BEVEL << SVG_QUOTE;
            break;
        case Qt::RoundJoin:
            qts << SVG_STROKE_LINEJOIN   << SVG_ROUND << SVG_QUOTE;
            break;
        default:
            qWarning("Unhandled join style");
            break;
        }
                if (spen.isCosmetic())
            qts << SVG_VECTOR_EFFECT;

        return qs;
    }

                const QString  qbrushToSvg(const QBrush &sbrush)
    {
        QString     qs;
        QTextStream qts(&qs);

        QString color, colorOpacity;

        switch (sbrush.style()) {
        case Qt::SolidPattern:
            translate_color(sbrush.color(), &color, &colorOpacity);

                        if (color != SVG_BLACK)
                qts << SVG_FILL << color << SVG_QUOTE;

                        if (colorOpacity != SVG_ONE)
                qts << SVG_FILL_OPACITY << colorOpacity << SVG_QUOTE;

            break;

        case Qt::NoBrush:
            qts << SVG_FILL << SVG_NONE <<  SVG_QUOTE;
            break;


        default:
           break;
        }
        return qs;
    }

};

class SvgGeneratorPrivate
{
public:
    SvgPaintEngine *engine;

    uint owns_iodevice : 1;
    QString fileName;
};




SvgGenerator::SvgGenerator()
    : d_ptr(new SvgGeneratorPrivate)
{
    Q_D(SvgGenerator);

    d->engine = new SvgPaintEngine;
    d->owns_iodevice = false;
}


SvgGenerator::~SvgGenerator()
{
    Q_D(SvgGenerator);
    if (d->owns_iodevice)
        delete d->engine->outputDevice();
    delete d->engine;
}


QString SvgGenerator::title() const
{
    Q_D(const SvgGenerator);

    return d->engine->documentTitle();
}

void SvgGenerator::setTitle(const QString &title)
{
    Q_D(SvgGenerator);

    d->engine->setDocumentTitle(title);
}


QString SvgGenerator::description() const
{
    Q_D(const SvgGenerator);

    return d->engine->documentDescription();
}

void SvgGenerator::setDescription(const QString &description)
{
    Q_D(SvgGenerator);

    d->engine->setDocumentDescription(description);
}


QSize SvgGenerator::size() const
{
    Q_D(const SvgGenerator);
    return d->engine->size();
}

void SvgGenerator::setSize(const QSize &size)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setSize(), cannot set size while SVG is being generated");
        return;
    }
    d->engine->setSize(size);
}


QRectF SvgGenerator::viewBoxF() const
{
    Q_D(const SvgGenerator);
    return d->engine->viewBox();
}


QRect SvgGenerator::viewBox() const
{
    Q_D(const SvgGenerator);
    return d->engine->viewBox().toRect();
}

void SvgGenerator::setViewBox(const QRectF &viewBox)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setViewBox(), cannot set viewBox while SVG is being generated");
        return;
    }
    d->engine->setViewBox(viewBox);
}

void SvgGenerator::setViewBox(const QRect &viewBox)
{
    setViewBox(QRectF(viewBox));
}


QString SvgGenerator::fileName() const
{
    Q_D(const SvgGenerator);
    return d->fileName;
}

void SvgGenerator::setFileName(const QString &fileName)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setFileName(), cannot set file name while SVG is being generated");
        return;
    }

    if (d->owns_iodevice)
        delete d->engine->outputDevice();

    d->owns_iodevice = true;

    d->fileName = fileName;
    QFile *file = new QFile(fileName);
    d->engine->setOutputDevice(file);
}


QIODevice *SvgGenerator::outputDevice() const
{
    Q_D(const SvgGenerator);
    return d->engine->outputDevice();
}

void SvgGenerator::setOutputDevice(QIODevice *outputDevice)
{
    Q_D(SvgGenerator);
    if (d->engine->isActive()) {
        qWarning("SvgGenerator::setOutputDevice(), cannot set output device while SVG is being generated");
        return;
    }
    d->owns_iodevice = false;
    d->engine->setOutputDevice(outputDevice);
    d->fileName = QString();
}


int SvgGenerator::resolution() const
{
    Q_D(const SvgGenerator);
    return d->engine->resolution();
}

void SvgGenerator::setResolution(int dpi)
{
    Q_D(SvgGenerator);
    d->engine->setResolution(dpi);
}


QPaintEngine *SvgGenerator::paintEngine() const
{
    Q_D(const SvgGenerator);
    return d->engine;
}


int SvgGenerator::metric(QPaintDevice::PaintDeviceMetric metric) const
{
    Q_D(const SvgGenerator);
    switch (metric) {
    case QPaintDevice::PdmDepth:
        return 32;
    case QPaintDevice::PdmWidth:
        return d->engine->size().width();
    case QPaintDevice::PdmHeight:
        return d->engine->size().height();
    case QPaintDevice::PdmDpiX:
        return d->engine->resolution();
    case QPaintDevice::PdmDpiY:
        return d->engine->resolution();
    case QPaintDevice::PdmHeightMM:
        return qRound(d->engine->size().height() / Ms::DPMM);
    case QPaintDevice::PdmWidthMM:
        return qRound(d->engine->size().width()  / Ms::DPMM);
    case QPaintDevice::PdmNumColors:
        return 0xffffffff;
    case QPaintDevice::PdmPhysicalDpiX:
        return d->engine->resolution();
    case QPaintDevice::PdmPhysicalDpiY:
        return d->engine->resolution();
    case QPaintDevice::PdmDevicePixelRatio:
    case QPaintDevice::PdmDevicePixelRatioScaled:
        return 1;
    default:
        qWarning("SvgGenerator::metric(), unhandled metric %d\n", metric);
        break;
    }
    return 0;
}


void SvgGenerator::setElement(const Ms::Element* e) {
    static_cast<SvgPaintEngine*>(paintEngine())->_element = e;
}



bool SvgPaintEngine::begin(QPaintDevice *)
{
    Q_D(SvgPaintEngine);

        if (!d->outputDevice) {
        qWarning("SvgPaintEngine::begin(), no output device");
        return false;
    }
    if (!d->outputDevice->isOpen()) {
        if (!d->outputDevice->open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning("SvgPaintEngine::begin(), could not open output device: '%s'",
                     qPrintable(d->outputDevice->errorString()));
            return false;
        }
    } else if (!d->outputDevice->isWritable()) {
        qWarning("SvgPaintEngine::begin(), could not write to read-only output device: '%s'",
                 qPrintable(d->outputDevice->errorString()));
        return false;
    }

        d->stream = new QTextStream(&d->header);
    stream() << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl << SVG_BEGIN;
    if (d->viewBox.isValid()) {
                stream() << SVG_WIDTH    << d->viewBox.width()  << SVG_PX << SVG_QUOTE
                 << SVG_HEIGHT   << d->viewBox.height() << SVG_PX << SVG_QUOTE;

        stream() << SVG_VIEW_BOX << d->viewBox.left()
                 << SVG_SPACE    << d->viewBox.top()
                 << SVG_SPACE    << d->viewBox.width()
                 << SVG_SPACE    << d->viewBox.height() << SVG_QUOTE << endl;
    }
    stream() << " xmlns=\"http:                " xmlns:xlink=\"http:                " version=\"1.2\" baseProfile=\"tiny\">" << endl;
    if (!d->attributes.title.isEmpty()) {
        stream() << SVG_TITLE_BEGIN << d->attributes.title.toHtmlEscaped() << SVG_TITLE_END << endl;
    }
    if (!d->attributes.description.isEmpty()) {
        stream() << SVG_DESC_BEGIN  << d->attributes.description.toHtmlEscaped() << SVG_DESC_END << endl;
    }


        d->stream->setString(&d->body);
    return true;
}

bool SvgPaintEngine::end()
{
    Q_D(SvgPaintEngine);


        d->stream->setDevice(d->outputDevice);

#ifndef QT_NO_TEXTCODEC
    d->stream->setCodec(QTextCodec::codecForName("UTF-8"));
#endif

        stream() << d->header;
    stream() << d->body;
    stream() << SVG_END << endl;

    delete d->stream;
    return true;
}

void SvgPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm,
                                 const QRectF &sr)
{
    drawImage(r, pm.toImage(), sr);
}

void SvgPaintEngine::drawImage(const QRectF  &r, const QImage &image,
                               const QRectF &sr, Qt::ImageConversionFlag flags)
{
    Q_UNUSED(sr);
    Q_UNUSED(flags);

    stream() << SVG_IMAGE           << stateString
             << SVG_X << SVG_QUOTE  << r.x() + _dx << SVG_QUOTE
             << SVG_Y << SVG_QUOTE  << r.y() + _dy << SVG_QUOTE
             << SVG_WIDTH           << r.width()   << SVG_QUOTE
             << SVG_HEIGHT          << r.height()  << SVG_QUOTE
             << SVG_PRESERVE_ASPECT << SVG_NONE    << SVG_QUOTE;

    QByteArray      data;
    QBuffer buffer(&data);

    buffer.open(QBuffer::ReadWrite);
    image.save(&buffer, "PNG");
    buffer.close();

    stream() << " xlink:href=\"data:image/png;base64,"
             << data.toBase64() << SVG_QUOTE << SVG_ELEMENT_END << endl;
}

void SvgPaintEngine::updateState(const QPaintEngineState &state)
{
        stateString.clear();

    
        stateStream << SVG_CLASS << getClass(_element) << SVG_QUOTE;

        stateStream << qbrushToSvg(state.brush());
    stateStream <<   qpenToSvg(state.pen());

    if (!qFuzzyIsNull(state.opacity() - 1))
        stateStream << SVG_OPACITY << state.opacity() << SVG_QUOTE;

                QTransform t = state.transform();

                    const qreal m11 = qRound(t.m11() * 1000) / 1000.0;
    const qreal m22 = qRound(t.m22() * 1000) / 1000.0;

    if (m11 == 1 && m22 == 1         && t.m12() == t.m21()) {                     _dx = t.m31();
          _dy = t.m32();
    }
    else {
                    _dx = 0;
          _dy = 0;
          stateStream << SVG_MATRIX << t.m11() << SVG_COMMA
                                    << t.m12() << SVG_COMMA
                                    << t.m21() << SVG_COMMA
                                    << t.m22() << SVG_COMMA
                                    << t.m31() << SVG_COMMA
                                    << t.m32() << SVG_RPAREN_QUOTE;
    }

}

void SvgPaintEngine::drawPath(const QPainterPath &p)
{
    stream() << SVG_PATH << stateString;

            if (p.fillRule() == Qt::OddEvenFill)
        stream() << SVG_FILL_RULE;

        stream() << SVG_D;
    for (int i = 0; i < p.elementCount(); ++i) {
        const QPainterPath::Element &e = p.elementAt(i);
                               qreal x = e.x + _dx;
                               qreal y = e.y + _dy;
        switch (e.type) {
        case QPainterPath::MoveToElement:
            stream() << SVG_MOVE  << x << SVG_COMMA << y;
            break;
        case QPainterPath::LineToElement:
            stream() << SVG_LINE  << x << SVG_COMMA << y;
            break;
        case QPainterPath::CurveToElement:
            stream() << SVG_CURVE << x << SVG_COMMA << y;
            ++i;
            while (i < p.elementCount()) {
                const QPainterPath::Element &e = p.elementAt(i);
                if (e.type == QPainterPath::CurveToDataElement) {
                    stream() << SVG_SPACE << e.x + _dx
                             << SVG_COMMA << e.y + _dy;
                    ++i;
                }
                else {
                    --i;
                    break;
                }
            }
            break;
        default:
            break;
        }
        if (i <= p.elementCount() - 1)
            stream() << SVG_SPACE;
    }
    stream() << SVG_QUOTE << SVG_ELEMENT_END << endl;
}

void SvgPaintEngine::drawPolygon(const QPointF *points, int pointCount,
                                  PolygonDrawMode mode)
{
    Q_ASSERT(pointCount >= 2);

    QPainterPath path(points[0]);
    for (int i=1; i<pointCount; ++i)
        path.lineTo(points[i]);

    if (mode == PolylineMode) {
        stream() << SVG_POLYLINE << stateString
                 << SVG_POINTS;
        for (int i = 0; i < pointCount; ++i) {
            const QPointF &pt = points[i];
            stream() << pt.x() + _dx << SVG_COMMA << pt.y() + _dy;
            if (i != pointCount - 1)
                stream() << SVG_SPACE;
        }
        stream() << SVG_QUOTE << SVG_ELEMENT_END <<endl;
    }
    else {
        path.closeSubpath();
        drawPath(path);
    }
}
