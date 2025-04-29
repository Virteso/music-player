#include <iostream>
#include <vlc/vlc.h>

int main(int argc, char* argv[]) {
	const char* filepath;
	if (argc != 2) {
		std::cout << "Usage: " << argv[0] << " <audio_file_path>" << std::endl;
		filepath = "../test.mp3";
	} else {
		filepath = argv[1];
	}

	// Initialize libVLC
	libvlc_instance_t* vlcInstance = libvlc_new(0, nullptr);
	if (!vlcInstance) {
		std::cerr << "Failed to create libVLC instance." << std::endl;
		return 1;
	}

	// Create a media
	libvlc_media_t* media = libvlc_media_new_path(filepath);
	if (!media) {
		std::cerr << "Failed to create media from path." << std::endl;
		libvlc_release(vlcInstance);
		return 1;
	}

	// Create a media player
	libvlc_media_player_t* mediaPlayer = libvlc_media_player_new_from_media(vlcInstance, media);
	libvlc_media_release(media);

	if (!mediaPlayer) {
		std::cerr << "Failed to create media player." << std::endl;
		libvlc_release(vlcInstance);
		return 1;
	}

	// Start playing
	if (libvlc_media_player_play(mediaPlayer) != 0) {
		std::cerr << "Failed to play media." << std::endl;
		libvlc_media_player_release(mediaPlayer);
		libvlc_release(vlcInstance);
		return 1;
	}

	std::cout << "Playing: " << filepath << std::endl;
	std::cout << "Controls: [p]ause/resume, [q]uit" << std::endl;

	char command;
	while (std::cin >> command) {
		if (command == 'p') {
			libvlc_media_player_pause(mediaPlayer);
			std::cout << (libvlc_media_player_is_playing(mediaPlayer) ? "Paused" : "Resumed") << std::endl;
		} else if (command == 'q') {
			std::cout << "Quitting..." << std::endl;
			break;
		}
	}

	// Clean up
	libvlc_media_player_stop_async(mediaPlayer);
	libvlc_media_player_release(mediaPlayer);
	libvlc_release(vlcInstance);

	return 0;
}
