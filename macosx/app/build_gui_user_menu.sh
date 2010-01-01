#!/bin/sh

# Build addon menu files, from the global /Library/GRASS/$GRASS_MMVER/Modules
# and the user's $HOME/Library/GRASS/$GRASS_MMVER/Modules.

# test files to make sure they are appropriate for adding to the GUI menu.
# Using 'file', assume executable binaries OK.  Check scripts to see if they
# have GRASS options configured - a simple grep for #%Module.
# Other script languages may need their own test.

# addon commands can't have spaces in them or sh for loop messes up.
# may be my limited knowledge of sh scripting and there could be a way.

GRASS_MMVER=`cut -d . -f 1-2 "$GISBASE/etc/VERSIONNUMBER"`
BINDIR="$GISBASE_USER/Modules/bin"
BINDIRG="$GISBASE_SYSTEM/Modules/bin"
MENUDIR="$GISBASE_USER/Modules/etc"

echo "Rebuilding Addon menu..."

# just to make sure (startup should have created it)
mkdir -p "$MENUDIR"
echo "# generated by grass startup" > "$MENUDIR/xtnmenu.dat"

# global addons:
if [ -d "$BINDIRG" ] ; then
  cd "$BINDIRG"
  CMDLISTG=`ls -1 2> /dev/null | sort -u`
else
  CMDLISTG=""
fi
CMDGFOUND=""

if [ "$CMDLISTG" != "" ] ; then
  for i in $CMDLISTG
  do
    ftype="`file "$BINDIRG/$i"`"
    if [ "`echo $ftype | grep 'Mach-O'`" ] || [ "`grep '#% *Module' "$BINDIRG/$i"`" ] ; then
      echo "main:$i:$i:$i" >> "$MENUDIR/xtnmenu.dat"
      CMDGFOUND="1"
    fi
  done
fi

# user addons:
CMDFIRST="1"
cd "$BINDIR"
CMDLIST=`ls -1 2> /dev/null | sort -u`

if [ "$CMDLIST" != "" ] ; then
  for i in $CMDLIST
  do
    ftype="`file "$BINDIR/$i"`"
    if [ "`echo $ftype | grep 'Mach-O'`" ] || [ "`grep '#% *Module' "$BINDIR/$i"`" ] ; then
      if [ "$CMDFIRST" ] && [ "$CMDGFOUND" ] ; then
        echo "separator" >> "$MENUDIR/xtnmenu.dat"
        CMDFIRST=""
      fi
      echo "main:$i:$i:$i" >> "$MENUDIR/xtnmenu.dat"
    fi
  done
fi
