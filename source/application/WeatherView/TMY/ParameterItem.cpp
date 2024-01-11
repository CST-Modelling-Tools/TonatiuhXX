#include "ParameterItem.h"

#include <QRegularExpression>

#include "SunPath/math/math.h" // for degree


ParameterAbstract::ParameterAbstract(const QString& name, const QString& text, bool editable):
    QStandardItem(text),
    m_name(name)
{
    setEditable(editable);
}


QString formatInt(QString format, int v) // "d", "02d"
{
    if (format.isEmpty()) return "";
    int w = 0;
    QChar p = ' ';
    if (format[0] == '0') {
        w = format.mid(0, format.indexOf("d")).toInt();
        p = '0';
    }
    return QString("%1").arg(v, w, 10, p);
}

QString formatDouble(QString format, double v) // ".3f", ".6g"
{
    if (format.isEmpty()) return "";
    int a = format.indexOf(".");
    int p = format.mid(a + 1, 1).toInt();
    char f = format.mid(a + 2, 1).toInt();
    return QString::number(v, f, p);
}

ParameterDouble::ParameterDouble(const QString& name, double value, QString quantity, QString format):
    ParameterAbstract(name, "", true),
    m_quantity(quantity),
    m_format(format)
{
    setValue(value);
}

void ParameterDouble::setValue(double value)
{
    m_value = value;
    QString text = toString(value);
    setText(text);
}

void ParameterDouble::setData(const QVariant& value, int role)
{
    if (role == Qt::EditRole) {
        QString t = value.toString();
        if (t == text()) return;
        double v = fromString(t);
        setValue(v);
        return;
    }
    QStandardItem::setData(value, role);
}

QString ParameterDouble::toString(double value)
{
    if (m_quantity == "time") {
        int m = value/60;
        int h = m/60;
        int d = h/24;
        m -= h*60;
        h -= d*24;

        QString ans = m_format;

        static QRegularExpression refd("{([^}]+)}d"); // any char except[^}] to avoid nesting
        QRegularExpressionMatch match = refd.match(m_format);
        QString fd = formatInt(match.captured(1), d) + "d";
        ans.replace(refd, fd);

        static QRegularExpression refh("{([^}]+)}h"); // static warning
        match = refh.match(m_format);
        QString fh = formatInt(match.captured(1), h) + "h";
        ans.replace(refh, fh);

        static QRegularExpression refm("{([^}]+)}m");
        match = refm.match(m_format);
        QString fm = formatInt(match.captured(1), m) + "m";
        ans.replace(refm, fm);

        return ans;
    }

    static QRegularExpression re("{(?<format>.+)}(?<unit>.*)");
    QRegularExpressionMatch match = re.match(m_format);
    if (!match.hasMatch()) {
        return "";
    }
    QString format = match.captured("format");
    QString unit = match.captured("unit");

    if (m_quantity == "") {

    } else if (m_quantity == "angle") { // "{.4f} deg", "{.4f} rad"
        if (unit.contains("deg"))
            value /= sp::degree;
    } else if (m_quantity == "energy") {
        if (unit.contains("kWh"))
            value /= 1000*3600;
    }
    return formatDouble(format, value) + unit;
}

double ParameterDouble::fromString(QString text)
{
    if (m_quantity == "") {
        double value = text.toDouble();
        return value;
    } else if (m_quantity == "angle") { // "{.4f} deg", "{.4f} rad"
        static QRegularExpression re("(?<value>.+)(?<unit>deg|rad)");
        QRegularExpressionMatch match = re.match(text);
        double value = match.captured("value").toDouble();
        QString unit = match.captured("unit");
        if (unit.contains("deg"))
            value *= sp::degree;
        return value;
    } else if (m_quantity == "energy") {
        static QRegularExpression re("(?<value>.+)(?<unit>kWh)");
        QRegularExpressionMatch match = re.match(text);
        double value = match.captured("value").toDouble();
        QString unit = match.captured("unit");
        if (unit.contains("kWh"))
            value *= 1000*3600;
        return value;
    } else if (m_quantity == "time") {
        static QRegularExpression re("((?<d>.*)d)?((?<h>.*)h)?(?<m>.*)m");
        QRegularExpressionMatch match = re.match(text);
        int d = match.captured("d").toInt();
        int h = match.captured("h").toInt();
        int m = match.captured("m").toInt();
        int s = ((d*24 + h)*60 + m)*60;
        return s;
    }
    return 0.;
}



