from functools import cache
from math import pow

from mido import MidiFile, tick2second


class Converter:
    def __init__(self, file_name: str) -> None:
        self.midi = MidiFile(file_name)
    
    @cache
    def midi_num_to_freq(self, midi_num: int) -> float:
        return 440 * pow(2, (midi_num - 69) / 12)
    
    def convert(self) -> tuple[list[float], list[float]]:
        meta, track = self.midi.tracks

        tempo = 0
        for msg in meta:
            if msg.type == "set_tempo":
                tempo = msg.tempo

        notes_freq = []
        notes_duration = []
        for msg in track:
            if msg.type == "note_on":
                notes_freq.append(self.midi_num_to_freq(msg.note))
            if msg.type == "note_off":
                notes_duration.append(tick2second(msg.time, self.midi.ticks_per_beat, tempo) * 1000)

        return notes_freq, notes_duration
    
    def convert_to_cpp(self) -> str:
        freqs, durs = self.convert()
        freqs = ", ".join(f"{round(note)}" for note in freqs)
        durs = ", ".join(f"{round(dur)}" for dur in durs)
        return f"constexpr uint32_t freqs[] = {{{freqs}}};\nconstexpr int32_t durs[] = {{{durs}}};\nconstexpr int32_t music_size = {len(freqs)};\n"


def main():
    converter = Converter("pirates.mid")
    print(converter.convert_to_cpp())


if __name__ == "__main__":
    main()
