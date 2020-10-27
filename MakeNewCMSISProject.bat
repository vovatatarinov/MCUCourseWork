@set /p NewProjectName=Enter a name for the new project: 

@echo Copy files...
@xcopy Templates\CMSISProjectTemplate\* Projects\%NewProjectName%\* /s /e

@echo Set correct names.
@for /R "Projects\%NewProjectName%" %%x in (CMSISProjectTemplate.*) do @ren %%x %NewProjectName%.*

@echo Project %NewProjectName% created in Projects directory.
@echo Add this project to existing IAR workspace manually.
@pause 