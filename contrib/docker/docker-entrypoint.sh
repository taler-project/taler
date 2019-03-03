#!/bin/sh
set -e

if [ $(echo "$1" | cut -c1) = "-" ]; then
  echo "$0: assuming arguments for talerd"

  set -- talerd "$@"
fi

if [ $(echo "$1" | cut -c1) = "-" ] || [ "$1" = "talerd" ]; then
  mkdir -p "$TALER_DATA"
  chmod 700 "$TALER_DATA"
  chown -R taler "$TALER_DATA"
  # echo "$0: setting data directory to $TALER_DATA"
  set -- "$@" -datadir="$TALER_DATA"
fi

if [ "$1" = "talerd" ] || [ "$1" = "taler-cli" ] || [ "$1" = "taler-tx" ]; then
  echo
  exec su-exec taler "$@"
fi

echo
exec "$@"
