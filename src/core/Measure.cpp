#include <core/Measure.hpp>

MeasureMap::MeasureMap() {
    _t_sigs.push_back({ 0, 4, 4, 0 });
}

MBT MeasureMap::GetMBT(int tick)
{
    const auto t_sig = GetTimeSig(tick);
    const int tpb = 480 * (4/t_sig.denominator); // Ticks per beat
    const int tpm = tpb * t_sig.numerator; // Ticks per measure
    const int t_delta = tick - t_sig.tick; // Tick delta

    const int m_diff = t_delta / tpm; // How many measures happened between ticks
    const int m_tick = t_delta % tpm; // How many ticks we are into the current measure

    return {t_sig.measure + m_diff, m_tick / 480, m_tick % 480};
}

void MeasureMap::AddTimeSig(int measure, int numerator, int denominator) {
    if (measure == 0 && _t_sigs.size() == 1) {
        _t_sigs[0] = { 0, numerator, denominator, 0 };
    } else {
        const auto& last_sig = _t_sigs.back();
        const int new_tick = last_sig.tick + (last_sig.numerator * (measure - last_sig.measure) * 1920) / last_sig.denominator;
        _t_sigs.push_back({ measure, numerator, denominator, new_tick });
    }
}

TimeSigChange MeasureMap::GetTimeSig(int tick) {
    int i = 0;
    while (i < _t_sigs.size()) {
        const auto& t_sig = _t_sigs[i];
        if (t_sig.tick > tick) {
            break;
        }
        i += 1;
    }

    if (i != 0) {
        i -= 1;
    }

    return _t_sigs[i];
}
