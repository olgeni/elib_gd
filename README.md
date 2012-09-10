elib_gd
=======

A simple Erlang interface to GD, developed and tested on FreeBSD.

This is a preliminary release, pending a general cleanup. However, you
can already generate graphics on Erlang using the full power of GD.

A few helper functions were added to deal with...

- Actual text metrics;

- Some obscure requirements related to ETSI 300 743 that you probably
  are not interested in.

If you like Operas, maybe you had a chance to see this library in action
on the big screen :)

If you note any missing GD function please add it and contribute, as I
haven't been following changes in GD for a while.

Todo
====

- Use NIFs (back then we didn't have NIFs, only 0s and 1s).

- Add some much needed documentation.

- Check Linux compatibility (mostly build paths).

<!-- Local Variables: -->
<!-- fill-column: 75 -->
<!-- End: -->
