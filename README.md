# raru: run as random user

Does it concern you if your browser can read your SSH keys? Do you think that on
most personal systems, a user account is equally valuable as root?

You can talk endlessly about secure coding practices. Ultimately, your
applications probably have holes, your system probably has holes, and your
kernel probably has holes. For the most part, the problems get progressively
more dangerous the further you get down the stack, but they start with the user
account. If you run two processes as one user and one of them is hacked, it can
generally run amok and do whatever it wants to to the other process.

Good ways to thwart this:

1. Capsicum. It's in FreeBSD, pretty much a system call (`cap_enable()`) to drop
the current processes' privileges. This seems to work great for applications you
are coding and has already been done to some of the userland utilities on
FreeBSD. So in a nutshell, if you are running `gzip` in a pipe, it doesn't need
to access files other than it's open file descriptors to stdin, stdout, and
stderr. You can either open a file for reading or pretty much immediately drop
its privileges before the exciting code comes into place, where there's more
likely to be some sort of buffer overflow. So, if you're writing something where
this is relevant, use it even if you have to `#ifdef` it.

2. Jails, containers, or VMs. Not the lightest thing around, but you can make it
work. You do have a lot of overhead having to maintain two "systems" and make
sure what you want is on the target. This can be scripted, automated, and
cleaned up, but may always be a bit overkill for some things. And as always, the
easier security is, the more you'll do it.

3. Run a process as a different user ID. As long as your home is `chmod 700`ed,
this pretty much drops off the front door entrace to your SSH keys, your
cookies, your AWS keys, etc. This also is generally clunky and is not used as
much as it should.

So, `raru`. A user account doesn't need to have an `/etc/passwd` or an
`/etc/shadow` entry to be used. It will simply pick random UIDs and they will
show up in `ps` as that UID, not as some username.

One interesting behavior: If you `setuid()` in a folder tree of
`/chmod700/chmod755`, you can poke around in `.`, even if you could never get
there normally. `raru` will work with local paths if it has explict access to
the current working directory. If it does not, it changes its directory to `/`.

`raru` effectively wraps your command and arguments. Unfortunately, it has to be
setuid to work. It sets a random UID and GID (both to the same) and runs the
program. `raru` will try to to stay in `.` if 'everyone' has explict access to
it. It will attempt to `chdir(getcwd())`, effectively, and if that fails it will
`chdir("/")`, instead. This means that if you are working out of a home folder,
`raru` shouldn't be able to access any of the files. You will have to copy them,
perhaps, into `/tmp`, and then run `raru` from there. If you are unsure of
`raru`'s cwd behavior, run `pwd` and compare with `raru pwd`.

Usage:

`raru [program] <arguments>`

NOTE: `raru` does pass along the whole environment. So run `raru env` to see
what this is. If you don't like that, run `env -i PATH=$PATH raru`. You should
probably set that as an alias.

### Potential use examples:

`raru ./fishy-app-youve-never-ran-before`

`raru bash # Whole shell as a random user.`

`raru curl https://fishysite`

## xraru

Browsers are almost certainly the easiest way into your system. With Javascript,
optional plugins, and millions of lines required to do anything and even more to
do it well, they are complicated. If they have working file dialogs beyond one
directory, they probably aren't sandboxed fully. An attacker doesn't need to
break your VPN, find an root-access remote hole, and then poke around your
infrastructure. They just need to send you a link, compromise your system, and
own you from there.

`xraru` makes this slightly less bad by sandboxing the application with `Xvnc`
and connecting to it with `vncviewer`. It's slower, not resizable, not convient,
and currently only supports one application at a time. It needs a lot of work.
But, `xraru chrome` should load up Chrome and you can start poking away, and
only worry about the dozens of millions of lines of code beneath on the system
down, and not the dozens of millions of lines from the user up. Please note that
copy/paste buffers are sort of transparently handled, [possibly only shared when
the window is given focus]
(https://github.com/TurboVNC/tightvnc/blob/master/vnc_unixsrc/vncviewer/README#L45).
This may or may not be secure enough for you. If you have passwords touch your
copy/paste buffers, you should be concerned.

We have to start somewhere, right?

Usage:

`xraru [X program] <arguments>`

`xraru` only sets PATH and HOME environment variables. HOME is set to a
temporary directory that should be cleaned up afterwards. Cleanup is buggy (best
to kill with ctrl+c, not X'ing out of Chrome). Only one VNC connection is
allowed, so one compromised can't spy on the others, hopefully.

If you have `autocutsel` installed, it should help manage copy/paste buffers on
the sandboxed side. If you have `dwm` installed, it will run and probably make your
experience slightly better.

### Potential use examples

0. `xraru chrome`
0. `xraru tor-browser`
0. `xraru thunderbird # Except you have to resetup your account every time.`
0. `xraru some-game-probably-not-in-3d`
0. `xraru sxiv defconownsyou.jpg`
0. `xraru openoffice memo.ppx`
0. `xraru wine TA.exe`
0. `xraru QT_X11_NO_MITSHM=1 pybitmessage`

### Dependencies

* /bin/sh
* tightvnc
* Optional: autocutsel, dwm

## Building and installing

`$ make`

`# make install # THIS INSTALLS AS SETUID. THINK TWICE AND READ THE CODE.`

## Gotchas

These are probably far more than listed. However, at least be aware of the
following:

0. You probably want to `alias raru='env -i PATH=$PATH raru'`
0. `raru`, to work, is setuid
0.  Run `raru whoami` a couple times to be certain that it's working.
0. `raru ls .` does not work if 'everyone' does not have access to `.`, anywhere from `/` on up.

## License

[Unlicense](LICENSE)/Public domain
