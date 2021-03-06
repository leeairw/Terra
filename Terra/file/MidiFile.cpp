#include "MidiFile.hpp"
#include <MidiFile.h>
#include <fstream>
#include "../misc/StrCnv.hpp"

NS_HWM_BEGIN

std::vector<SequencePtr> CreateSequenceFromSMF(String path)
{
    smf::MidiFile smf;
    
    std::ifstream ifs(to_utf8(path).c_str());

    bool successful = smf.read(ifs);
    if(!successful) {
        hwm::wdout << L"Failed to load {}"_format(path) << std::endl;
        return {};
    }
    
    smf.doTimeAnalysis();
    smf.linkNotePairs();
    
    std::vector<SequencePtr> ss;
    
    auto const num_tracks = smf.getTrackCount();
    
    for(int tn = 0; tn < num_tracks; ++tn) {
        ss.push_back(std::make_unique<Sequence>());
        auto &seq = ss[tn];
        auto const num_events = smf.getNumEvents(tn);
        for(int en = 0; en < num_events; ++en) {
            auto const &event = smf.getEvent(tn, en);
            if(event.isNoteOn() && event.isLinked()) {
                auto *linked = event.getLinkedEvent();
                auto note = std::make_shared<Sequence::Note>();
                note->pos_ = event.tick;
                note->length_ = linked->tick;
                note->pitch_ = event[1];
                note->velocity_ = event[2];
                seq->notes_.push_back(std::move(note));
            } else if(event.isTrackName()) {
                auto clone = event;
                seq->name_ = to_wstr(clone.getMetaContent());
            }
        }
    }
    
    return ss;
}

NS_HWM_END
