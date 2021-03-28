# from `niv show nixpkgs`
# nixops modify -d rpi nixops.nix -I nixpkgs=https://github.com/NixOS/nixpkgs/archive/45e01e508a792077c77067467e07bce757932fb6.tar.gz
{
  network.enableRollback = true;
  rpi = {
    imports = [
      ./nixos-with-fs.nix
    ];
    deployment.targetHost = "192.168.2.177";
  };
}
