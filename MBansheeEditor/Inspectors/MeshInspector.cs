﻿using System.Collections.Generic;
using BansheeEngine;

namespace BansheeEditor
{
    /// <summary>
    /// Renders an inspector for the <see cref="Mesh"/> resource.
    /// </summary>
    [CustomInspector(typeof(Mesh))]
    internal class MeshInspector : Inspector
    {
        private GUIToggleField normalsField = new GUIToggleField(new LocEdString("Import Normals"));
        private GUIToggleField tangentsField = new GUIToggleField(new LocEdString("Import Tangents"));
        private GUIToggleField skinField = new GUIToggleField(new LocEdString("Import Skin"));
        private GUIToggleField blendShapesField = new GUIToggleField(new LocEdString("Import Blend Shapes"));
        private GUIToggleField animationField = new GUIToggleField(new LocEdString("Import Animation"));
        private GUIFloatField scaleField = new GUIFloatField(new LocEdString("Scale"));
        private GUIToggleField cpuReadableField = new GUIToggleField(new LocEdString("CPU readable"));
        private GUIButton reimportButton = new GUIButton(new LocEdString("Reimport"));

        private MeshImportOptions importOptions;

        /// <inheritdoc/>
        protected internal override void Initialize()
        {
            if (referencedObject != null)
            {
                importOptions = GetImportOptions();

                normalsField.OnChanged += x => importOptions.ImportNormals = x;
                tangentsField.OnChanged += x => importOptions.ImportTangents = x;
                skinField.OnChanged += x => importOptions.ImportSkin = x;
                blendShapesField.OnChanged += x => importOptions.ImportBlendShapes = x;
                animationField.OnChanged += x => importOptions.ImportAnimation = x;
                scaleField.OnChanged += x => importOptions.Scale = x;
                cpuReadableField.OnChanged += x => importOptions.CPUReadable = x;

                reimportButton.OnClick += TriggerReimport;

                layout.AddElement(normalsField);
                layout.AddElement(tangentsField);
                layout.AddElement(skinField);
                layout.AddElement(blendShapesField);
                layout.AddElement(animationField);
                layout.AddElement(scaleField);
                layout.AddElement(cpuReadableField);
                layout.AddSpace(10);

                GUILayout reimportButtonLayout = layout.AddLayoutX();
                reimportButtonLayout.AddFlexibleSpace();
                reimportButtonLayout.AddElement(reimportButton);
            }
        }

        /// <inheritdoc/>
        protected internal override void Refresh()
        {
            MeshImportOptions newImportOptions = GetImportOptions();

            normalsField.Value = newImportOptions.ImportNormals;
            tangentsField.Value = newImportOptions.ImportTangents;
            skinField.Value = newImportOptions.ImportSkin;
            blendShapesField.Value = newImportOptions.ImportBlendShapes;
            animationField.Value = newImportOptions.ImportAnimation;
            scaleField.Value = newImportOptions.Scale;
            cpuReadableField.Value = newImportOptions.CPUReadable;

            importOptions = newImportOptions;
        }


        /// <summary>
        /// Retrieves import options for the mesh we're currently inspecting.
        /// </summary>
        /// <returns>Mesh import options object.</returns>
        private MeshImportOptions GetImportOptions()
        {
            Mesh mesh = referencedObject as Mesh;
            MeshImportOptions output = null;

            if (mesh != null)
            {
                LibraryEntry meshEntry = ProjectLibrary.GetEntry(ProjectLibrary.GetPath(mesh));
                if (meshEntry != null && meshEntry.Type == LibraryEntryType.File)
                {
                    FileEntry meshFileEntry = (FileEntry)meshEntry;
                    output = meshFileEntry.Options as MeshImportOptions;
                }
            }

            if (output == null)
            {
                if (importOptions == null)
                    output = new MeshImportOptions();
                else
                    output = importOptions;
            }

            return output;
        }

        /// <summary>
        /// Reimports the texture resource according to the currently set import options.
        /// </summary>
        private void TriggerReimport()
        {
            Mesh mesh = (Mesh)referencedObject;
            string resourcePath = ProjectLibrary.GetPath(mesh);

            ProjectLibrary.Reimport(resourcePath, importOptions, true);
        }
    }
}