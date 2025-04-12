#!/usr/bin/env python3
import subprocess
import os

# Input and output paths
# input_video = "./testdata/qt_trim.mov"
# input_video = "./indata/pacpac.mov"
# input_video = "./indata/ropasci.mov"
#input_video = "./indata/coke.mov"
input_video = "./indata/apesky.mov"

output_dir = "./outdata/"

# Quality settings (treat these as target video bitrates in kbps)
#qualities = [500,750]
#qualities = [500,750]
#qualities = [750]
#qualities = [900]
qualities = [1000,1200,1400,2000]

# Output resolution and FPS settings
#resolutions = [128, 256, 512]
#resolutions = [128, 256, 512]
resolutions = [256,512]

# Frames per second
# fps_values = [15, 20, 30, 60]
#fps_values = [30, 60]
fps_values = [60]

# Start and end time for trimming (in milliseconds)
start_time_ms = 0
#end_time_ms = 12000*2
end_time_ms = 8000

# Playback speed factor (e.g., 2.0 for 2x speed)
speed_factor = 1.2
#speed_factor = 2.0
# speed_factor = 4.0

# Ensure output directory exists
os.makedirs(output_dir, exist_ok=True)

def convert_to_mp4(input_file, output_file, width, fps, quality, start_ms, end_ms, speed_factor):
    """Converts a video to MP4 (H.264 + AAC) format with specified settings,
       trims the video, and applies a speed adjustment."""
    # Convert milliseconds to time format (hh:mm:ss.sss)
    start_time = f"{start_ms // 3600000:02}:{(start_ms % 3600000) // 60000:02}:{(start_ms % 60000) / 1000:.3f}"
    end_time = f"{end_ms // 3600000:02}:{(end_ms % 3600000) // 60000:02}:{(end_ms % 60000) / 1000:.3f}"
    
    # Calculate the setpts value for the speed adjustment
    setpts_value = 1 / speed_factor

    command = [
        "ffmpeg",
        "-ss", start_time,                      # Start time
        "-to", end_time,                        # End time
        "-f", "lavfi", "-i", "anullsrc=cl=stereo:sample_rate=48000",
        "-i", input_file,
        "-vf", f"scale={width}:-2,setpts={setpts_value}*PTS",  # Adjust playback speed
        "-r", str(fps),                         # Set FPS
        "-c:v", "libx264",                      # Use H.264 codec
        "-profile:v", "baseline",               # Baseline profile for broader compatibility
        "-level:v", "3.0",                      # Level 3.0 for broader compatibility
        "-b:v", f"{quality}k",                  # Set video bitrate
        "-c:a", "aac",                          # Use AAC audio codec
        "-b:a", "128k",                         # Set audio bitrate
        "-movflags", "+faststart",              # Enable fast start for streaming
        "-af", f"atempo={speed_factor}",        # Adjust audio speed
        "-shortest",                            # Match length to shortest stream (video)
        "-map", "1:v:0",                        # Map video from second input (the actual video file)
        "-map", "0:a:0",                        # Map audio from first input (the nullsrc)
        "-y",                                   # Overwrite output file without asking
        output_file                             # Output file
    ]
    try:
        subprocess.run(command, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error during conversion: {e}")

def main():
    print("Starting video conversion...")
    for width in resolutions:
        for fps in fps_values:
            for quality in qualities:
                output_filename = f"qt_trim_{width}p_{fps}fps_q{quality}_speed{speed_factor}x.mp4"
                output_path = os.path.join(output_dir, output_filename)
                print(f"Converting with resolution {width}px, fps {fps}, quality {quality}kbps, speed {speed_factor}x -> {output_path}")
                try:
                    convert_to_mp4(input_video, output_path, width, fps, quality, start_time_ms, end_time_ms, speed_factor)
                    print(f"Successfully created: {output_path}")
                except subprocess.CalledProcessError as e:
                    print(f"Error during conversion with resolution {width}px, fps {fps}, quality {quality}: {e}")

if __name__ == "__main__":
    main()
