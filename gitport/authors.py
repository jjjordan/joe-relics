
jhallen = (b"Joe Allen", b"jhallenworld@gmail.com")
jhallen_old = (b"Joseph H. Allen", b"jhallen@world.std.com")
jjjordan = (b"John J. Jordan", b"jj@jjjordan.io")

hg_author_map = {
    (b"Joseph Allen", b"jhallenworld@gmail.com"): jhallen,
    (b"Joseph Allen", b"jhallen@theworld.com"): jhallen,
    (b"Joseph Allen", b"allenjo@us.ibm.com"): jhallen,
    (b"Joe Allen", b"jhallen@gmail.com"): jhallen,
    (b"John J. Jordan", b"jjjordan@users.sf.net"): jjjordan,
    (b"jjjordan", b"jjjordan@users.sf.net"): jjjordan,
}

cvs_author_map = {
    b"jhallen": jhallen,
    b"marx_sk": (b"Marek 'Marx' Grac", b"mgrac@redhat.com"),
    b"polesapart": (b"Alexandre P. Nunes", b"alexandre.nunes@gmail.com"),
    b"shallot": (b"Josip Rodin", b"joy+joe@entuzijast.net"),
    b"electrum": (b"David Phillips", b"david@acz.org"),
#    b"sonic_amiga": (b"Pavel Fedin", b"pavel_fedin@mail.ru"),
#    b"vsamel": (b"Vitezslav Samel", b"vitezslav@samel.cz"),
# Old:
    b"sonic_amiga": (b"Pavel Fedin", b"sonic_amiga@rambler.ru"),
    b"vsamel": (b"Vitezslav Samel", b"samel@mail.cz"),
}
