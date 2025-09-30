#pragma once
#include <array>

enum class TrackDifficulty : int { Easy, Medium, Hard, Expert, Count };

enum class SongDifficulty : int {
  Warmup,
  Apprentice,
  Solid,
  Moderate,
  Challenging,
  Nightmare,
  Impossible,
  Count
};

enum class SongGenre {
  Alternative,
  Blues,
  Classic_Rock,
  Country,
  Emo,
  Fusion,
  Glam,
  Grunge,
  Indie_Rock,
  Jazz,
  Metal,
  Novelty,
  Nu_Metal,
  Pop_Rock,
  Prog,
  Punk,
  Rock,
  Southern_Rock,
  Urban,
  Other,
  Count
};

enum class GameTrack : int { Drum, Bass, Guitar, Vocal, Count };

enum class AudioTrack : int { Drum, Bass, Guitar, Vocal, Misc, Count };

enum class NoteColour : int { Red, Yellow, Green, Blue };

inline constexpr std::array<const char*,
                            static_cast<int>(TrackDifficulty::Count)>
    TrackDiffNames = {"Easy", "Medium", "Hard", "Expert"};

inline constexpr std::array<const char*,
                            static_cast<int>(SongDifficulty::Count)>
    SongDiffNames = {"Warmup",      "Apprentice", "Solid",     "Moderate",
                     "Challenging", "Nightmare",  "Impossible"};

inline constexpr std::array<const char*, static_cast<int>(SongGenre::Count)>
    SongGenreNames = {
        "Alternative", "Blues",   "Classic Rock",  "Country",    "Emo",
        "Fusion",      "Glam",    "Grunge",        "Indie Rock", "Jazz",
        "Metal",       "Novelty", "Nu Metal",      "Pop Rock",   "Prog",
        "Punk",        "Rock",    "Southern Rock", "Urban",      "Other",
};

inline constexpr std::array<const char*, static_cast<int>(GameTrack::Count)>
    GameTrackNames = {"Drums", "Bass", "Guitar", "Vocals"};

inline constexpr std::array<const char*, static_cast<int>(AudioTrack::Count)>
    AudioTrackNames = {"Drums", "Bass", "Guitar", "Vocals", "Misc"};

enum class ImportType : int { Midi, Count };
enum class ExportType : int { Rbu, Count };

inline constexpr std::array<const char*, static_cast<int>(ExportType::Count)>
    ExportTypes = {".rbu"};