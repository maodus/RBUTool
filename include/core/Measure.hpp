#pragma once
#include <vector>
#include <core/BinaryIO.hpp>

struct TimeSigChange {
    int measure;
    int numerator;
    int denominator;
    int tick;
};


// Measure/Beat/Tick
struct MBT {
    int measure;
    int beat;
    int tick;
};

class MeasureMap : IWritable {
private:
    std::vector<TimeSigChange> _t_sigs;
public:
    MeasureMap();
    MBT GetMBT(int tick);
    void AddTimeSig(int measure, int numerator, int denominator);
    TimeSigChange GetTimeSig(int tick);

    void write(BinaryWriter& w) const override {
        w.U32(_t_sigs.size());
        for (const auto& ts : _t_sigs) {
            w.U32(ts.measure);
            w.U32(ts.numerator);
            w.U32(ts.denominator);
            w.U32(ts.tick);
        }
    }
};