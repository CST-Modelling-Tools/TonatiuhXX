#pragma once

#include "SunPath/samplers/SunTemporal.h"

namespace sp {


struct SunPath ParamsTMY
{
    bool seconds = false; // write seconds
    bool adjustDay = false;
    int offset = 0; // in seconds
    int precision = 0; // precision for function
};


class SunPath FormatTMY
{
public:
    FormatTMY(SunTemporal* sunTemporal);

    bool read(QString fileName, const ParamsTMY& params = ParamsTMY());
    bool write(QString fileName, const ParamsTMY& params = ParamsTMY());
    QString message() const {return m_message;}

protected:
    void readInfo(QTextStream& fin, const ParamsTMY& params);
    void readData(QTextStream& fin, const ParamsTMY& params);

protected:
    SunTemporal* m_sunTemporal;
    QString m_message;
};


} // namespace sp
