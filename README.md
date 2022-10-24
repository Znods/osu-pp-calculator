# osu-pp-calculator twitch bot

This is an osu-pp-calculator bot that will display a beatmaps performance points in twitch chat.

- Replace token with your token in apiv2.c
- Replace oauth key with your key in main.c
- Also change botname to your twitch botname :)

# Dependencies
- apt install libjson-c-dev -y

# Compile
- gcc *.c -o pp -ljson-c -lm -trigraphs -DDEBUG

# Example Usage

Start bot:
- ./pp

Commands:
- !pp beatmap_id
- !pp beatmap_id +dt
- !pp beatmap_id +dthr
- !pp beatmap_id +dthd

https://user-images.githubusercontent.com/70346380/197395867-25361e75-e467-4a9f-9c62-fc502f8d2494.mp4
