# RBUTool
This an early access repo for a tool that allows for creating custom songs in Rock Band Unplugged for the PSP.

# Charts
The only currently support chart formats are midi charts from Rockband games. Additionally, the chart editor that is included in this tool is not a feature-complete replacement for standard chart editors. The editor is simply here to make small adjustments to your conversions.

# Audio
Before trying to convert audio tracks into .ATM files, please ensure that both [at3tool](https://www.pspunk.com/psp-atrac3/) and the latest version of [ffmpeg(full version)](https://ffmpeg.org/download.html) are added to your PATH environment variable.

Audio stems will be converted into an .ATM file, which can be used in the RBU. Please make sure to manually add the .ATM file extention before exporting.

# TODO
- Add input support for: gh(.mid), .chart, BEATMATCH.STR
- Add export support for: rb(.mid), gh(.mid), .chart
- Make overdrive and solo events visible in chart editor