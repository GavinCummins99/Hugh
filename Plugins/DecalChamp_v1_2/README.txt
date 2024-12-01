Thank you for downloading Decal Champ!  This tool is completely free to use for personal and professional projects!

*** PLEASE READ --- 

Decal Champ v1.2 has fairly major changes to the blueprint, folder organization and material functions.  If you are upgrading from v1.0 or v1.1, it's best to remove the old version of Decal Champ and replace fully with v1.2 as a lot has changed.  However, if you have a lot of Decal Champ decals in your scenes so you don't want to lose the previous Decal Champ decals then you can try just migrating v1.2 over your previous version in your project and just replacing all files when prompted.  I tested this and it seems to work fine but you will have some erroneous textures and materials in your Decal Champ folders that are no longer used or have moved in v1.2.  Use with care and always backup your projects first!!!  Decal Champ v1.2 is now only available for UE 5.1 and above. ***

v1.2 Update

New Features...
- Added new Triplanar and Parallax Occlusion Mapping (POM) Advanced Decals - please use with care as these can be expensive!
- Added new Emissive (Glow) parameter for both Primary and Secondary Texture and added 3 new profiles to demonstrate.  NOTE - This is not added to Triplanar (doesn't work), or POM (doesn't look good) decals
- Added some additional masks, textures

Optimizations...
- Updated Decal Blueprint so it holds materials as soft object references in an array and resolves them as it uses them.  This should prevent needing to keep all materials in memory
- Changed Profile Data Table and Textures within table to soft references so they only load when used
- Changed Scale condition in scale material function so it uses a lerp instead of an if statement to avoid branching and slightly improve performance
- Multiple folder and organization changes
- Demo Folder now contains all items only used in Demo scene - You can delete this folder to save on file space if you no longer need the demo scene 
- Increased Noise Contrast of Procedural mask max range from 10 to 20 (this can easily be adjusted in the Blueprint)
- Replaced brick texture that comes with Decal Champ as the previous texture was broken (highly recommend to use Megascans anyway)

-----------

v1.1 Update

You now have the option to set Metallic and Specular values on both primary and secondary textures.

Adding these settings required an update to the data table, so if you're updating from v1.0 then you'll need to recreate any custom profiles you made directly in the new table data.  As with all updates, it's highly recommended to backup your work first.

-----------

Enjoy!