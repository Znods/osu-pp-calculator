# osu-pp-calculator twitch bot

This is an osu-pp-calculator bot that will display a beatmaps performance points in twitch chat.

# Edit files
- Replace twitch oauth key with your key in main.c, line 25
- Change botname to your twitch botname in main.c, line 24 :)
- Replace osu client id & client secret in main.c, line 29 & 30 
- Change OWNER name to your twitch name in this format ":yourname:" in twitch.c, line 19

# Dependencies
- apt install libjson-c-dev -y

# Compile
- gcc *.c -o pp -ljson-c -lm -trigraphs -Wall -O2

DEBUG Mode:
- gcc *.c -o pp -ljson-c -lm -trigraphs -Wall -Og -DDEBUG

# Example Usage

Start bot:
- ./pp

Command Syntax:
- !pp beatmap_id
- !pp beatmap_id +dt
- !pp beatmap_id +dthr
- !pp beatmap_id +dthd
- !pp beatmap_id +dthrhd etc...

- !s - this will shutdown the bot
- !ms - see latency to twitch & osu

https://user-images.githubusercontent.com/70346380/197395867-25361e75-e467-4a9f-9c62-fc502f8d2494.mp4
