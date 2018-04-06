# Get the DevTools working on Android, no root needed

The Android version of Chrome is not compiled with the DevTools in it. See this Chrome Feature Request for more info.

These instructions let you use the DevTools anyway. It's not perfect, but does the job quite well : you'll get the inspector, console, debugger etc.

:warning: The hack consists of introducing a major security breach : all the sites you'll visit in the modified Chromium app will be accessible by all the apps on your system.


Remove Chromium if installed (not Chrome, Chromium). 

Install the APK published in the releases on this repository.

Run Chrome.
Run this Chromium.
Use the awesome Termux.

Add this to your ~/.bashrc :

```
function dt {
  local socketName=$(cat /proc/net/unix | grep -o 'chrome_devtools_remote_[0-9]*')
  socat -d -d tcp-listen:9222,fork,reuseaddr abstract-connect:$socketName
}
```

Run `dt` in your terminal.

Open localhost:9222 in Chrome, choose a tab, enjoy the DevTools.





# ![Logo](chrome/app/theme/chromium/product_logo_64.png) Chromium

Chromium is an open-source browser project that aims to build a safer, faster,
and more stable way for all users to experience the web.

The project's web site is https://www.chromium.org.

Documentation in the source is rooted in [docs/README.md](docs/README.md).

Learn how to [Get Around the Chromium Source Code Directory Structure
](https://www.chromium.org/developers/how-tos/getting-around-the-chrome-source-code).
