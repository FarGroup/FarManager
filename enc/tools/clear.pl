$dest_dr            = "../enc";
$dest_dr_inet       = $dest_dr."/inet";
$dest_dr_inet_ru    = $dest_dr_inet."/ru";
$dest_dr_inet_en    = $dest_dr_inet."/en";
$dest_dr_chm        = $dest_dr."/chm";
$dest_dr_chm_ru     = $dest_dr_chm."/ru";
$dest_dr_chm_en     = $dest_dr_chm."/en";

$meta_ru            = $dest_dr_chm_ru."/meta";
$meta_en            = $dest_dr_chm_en."/meta";

$toolpath           = "";
if($^O eq "MSWin32"){
  $toolpath="./tools/";
}

system $toolpath."rm -f -r ".$dest_dr;

print "done\n";
