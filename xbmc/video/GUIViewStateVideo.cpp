/*
 *      Copyright (C) 2005-2012 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIViewStateVideo.h"
#include "PlayListPlayer.h"
#include "filesystem/PluginDirectory.h"
#include "filesystem/PVRDirectory.h"
#include "filesystem/VideoDatabaseDirectory.h"
#include "filesystem/Directory.h"
#include "VideoDatabase.h"
#include "settings/GUISettings.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "FileItem.h"
#include "guilib/Key.h"
#include "guilib/LocalizeStrings.h"

using namespace XFILE;
using namespace VIDEODATABASEDIRECTORY;

CStdString CGUIViewStateWindowVideo::GetLockType()
{
  return "video";
}

CStdString CGUIViewStateWindowVideo::GetExtensions()
{
  return g_settings.m_videoExtensions;
}

int CGUIViewStateWindowVideo::GetPlaylist()
{
  return PLAYLIST_VIDEO;
}

VECSOURCES& CGUIViewStateWindowVideo::GetSources()
{
  AddLiveTVSources();
  return CGUIViewState::GetSources();
}

CGUIViewStateWindowVideoFiles::CGUIViewStateWindowVideoFiles(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS()); // Preformated
    AddSortMethod(SORT_METHOD_DRIVE_TYPE, 564, LABEL_MASKS()); // Preformated
    SetSortMethod(SORT_METHOD_LABEL);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderAscending);
  }
  else
  {
    if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
      AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty
    else
      AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty
    AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));  // Label, Size | Label, Size
    AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));  // Label, Date | Label, Date
    AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty

    SetSortMethod(g_settings.m_viewStateVideoFiles.m_sortMethod);
    SetViewAsControl(g_settings.m_viewStateVideoFiles.m_viewMode);
    SetSortOrder(g_settings.m_viewStateVideoFiles.m_sortOrder);
  }
  LoadViewState(items.GetPath(), WINDOW_VIDEO_FILES);
}

void CGUIViewStateWindowVideoFiles::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_FILES, &g_settings.m_viewStateVideoFiles);
}

VECSOURCES& CGUIViewStateWindowVideoFiles::GetSources()
{
  AddOrReplace(g_settings.m_videoSources, CGUIViewStateWindowVideo::GetSources());
  return g_settings.m_videoSources;
}

CGUIViewStateWindowVideoNav::CGUIViewStateWindowVideoNav(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Label, empty
    SetSortMethod(SORT_METHOD_NONE);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderNone);
  }
  else if (items.IsVideoDb())
  {
    NODE_TYPE NodeType=CVideoDatabaseDirectory::GetDirectoryChildType(items.GetPath());
    CQueryParams params;
    CVideoDatabaseDirectory::GetQueryParams(items.GetPath(),params);

    switch (NodeType)
    {
    case NODE_TYPE_MOVIES_OVERVIEW:
    case NODE_TYPE_TVSHOWS_OVERVIEW:
    case NODE_TYPE_MUSICVIDEOS_OVERVIEW:
    case NODE_TYPE_OVERVIEW:
      {
        AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Label, empty

        SetSortMethod(SORT_METHOD_NONE);

        SetViewAsControl(DEFAULT_VIEW_LIST);

        SetSortOrder(SortOrderNone);
      }
      break;
    case NODE_TYPE_DIRECTOR:
    case NODE_TYPE_ACTOR:
      {
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%R", "%L", ""));  // Title, Rating | Label, empty
        SetSortMethod(SORT_METHOD_LABEL);

        SetViewAsControl(g_settings.m_viewStateVideoNavActors.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavActors.m_sortOrder);
      }
      break;
    case NODE_TYPE_YEAR:
      {
        AddSortMethod(SORT_METHOD_LABEL, 562, LABEL_MASKS("%T", "%R", "%L", ""));  // Title, Rating | Label, empty
        SetSortMethod(SORT_METHOD_LABEL);

        SetViewAsControl(g_settings.m_viewStateVideoNavYears.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavYears.m_sortOrder);
      }
      break;
    case NODE_TYPE_SEASONS:
      {
        AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE, 551, LABEL_MASKS("%L", "","%L",""));  // Label, empty | Label, empty
        SetSortMethod(SORT_METHOD_VIDEO_SORT_TITLE);

        SetViewAsControl(g_settings.m_viewStateVideoNavSeasons.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavSeasons.m_sortOrder);
      }
      break;
    case NODE_TYPE_TITLE_TVSHOWS:
      {
        if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
          AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE, 551, LABEL_MASKS("%T", "%M", "%T", "%M"));  // Title, #Episodes | Title, #Episodes
        else
          AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE, 551, LABEL_MASKS("%T", "%M", "%T", "%M")); // Title, #Episodes | Title, #Episodes

        AddSortMethod(SORT_METHOD_EPISODE, 20360, LABEL_MASKS("%L", "%M", "%L", "%M"));  // Label, #Episodes | Label, #Episodes
        AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%L","%Y","%L","%Y")); // Label, Year | Label, Year
        SetSortMethod(SORT_METHOD_LABEL);

        SetViewAsControl(g_settings.m_viewStateVideoNavTvShows.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavTvShows.m_sortOrder);
      }
      break;
    case NODE_TYPE_MUSICVIDEOS_ALBUM:
    case NODE_TYPE_GENRE:
    case NODE_TYPE_COUNTRY:
    case NODE_TYPE_STUDIO:
      {
        AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%R", "%L", ""));  // Title, Rating | Label, empty
        SetSortMethod(SORT_METHOD_LABEL);

        SetViewAsControl(g_settings.m_viewStateVideoNavGenres.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavGenres.m_sortOrder);
      }
      break;
    case NODE_TYPE_SETS:
      {
        if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
          AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%T","%R", "%T","%R"));  // Title, Rating | Title, Rating
        else
          AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%R", "%T","%R"));  // Title, Rating | Title, Rating

        AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T", "%Y", "%T", "%Y"));  // Title, Year | Title, Year
        AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
        AddSortMethod(SORT_METHOD_DATEADDED, 570, LABEL_MASKS("%T", "%a", "%T", "%a"));  // Title, DateAdded | Title, DateAdded

        if (g_settings.GetWatchMode(items.GetContent()) == VIDEO_SHOW_ALL)
          AddSortMethod(SORT_METHOD_PLAYCOUNT, 576, LABEL_MASKS("%T", "%V", "%T", "%V"));  // Title, Playcount | Title, Playcount

        SetSortMethod(SORT_METHOD_LABEL_IGNORE_THE);

        SetViewAsControl(g_settings.m_viewStateVideoNavGenres.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavGenres.m_sortOrder);
      }
      break;
    case NODE_TYPE_TAGS:
      {
        SORT_METHOD method = SORT_METHOD_LABEL_IGNORE_THE;
        if (!g_guiSettings.GetBool("filelists.ignorethewhensorting"))
          method = SORT_METHOD_LABEL;

        AddSortMethod(method, 551, LABEL_MASKS("%T","", "%T",""));  // Title, empty | Title, empty
        SetSortMethod(method);
        SetViewAsControl(g_settings.m_viewStateVideoNavGenres.m_viewMode);
        SetSortOrder(g_settings.m_viewStateVideoNavGenres.m_sortOrder);
      }
      break;
    case NODE_TYPE_EPISODES:
      {
        if (params.GetSeason() > -1)
        {
          AddSortMethod(SORT_METHOD_EPISODE, 20359, LABEL_MASKS("%E. %T","%R"));  // Episode. Title, Rating | empty, empty
          AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%E. %T", "%R"));  // Episode. Title, Rating | empty, empty
          AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%E. %T", "%O"));  // Episode. Title, MPAA | empty, empty
          AddSortMethod(SORT_METHOD_PRODUCTIONCODE, 20368, LABEL_MASKS("%E. %T","%P", "%E. %T","%P"));  // Episode. Title, ProductionCode | Episode. Title, ProductionCode
          AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%E. %T","%J","%E. %T","%J"));  // Episode. Title, Date | Episode. Title, Date

          if (g_settings.GetWatchMode(items.GetContent()) == VIDEO_SHOW_ALL)
            AddSortMethod(SORT_METHOD_PLAYCOUNT, 576, LABEL_MASKS("%E. %T", "%V"));  // Episode. Title, Playcount | empty, empty
        }
        else
        {
          AddSortMethod(SORT_METHOD_EPISODE, 20359, LABEL_MASKS("%H. %T","%R"));  // Order. Title, Rating | emtpy, empty
          AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%H. %T", "%R"));  // Order. Title, Rating | emtpy, empty
          AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%H. %T", "%O"));  // Order. Title, MPAA | emtpy, empty
          AddSortMethod(SORT_METHOD_PRODUCTIONCODE, 20368, LABEL_MASKS("%H. %T","%P", "%H. %T","%P"));  // Order. Title, ProductionCode | Episode. Title, ProductionCode
          AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%H. %T","%J","%H. %T","%J"));  // Order. Title, Date | Episode. Title, Date

          if (g_settings.GetWatchMode(items.GetContent()) == VIDEO_SHOW_ALL)
            AddSortMethod(SORT_METHOD_PLAYCOUNT, 576, LABEL_MASKS("%H. %T", "%V"));  // Order. Title, Playcount | empty, empty
        }
        if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
          AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%T","%R"));  // Title, Rating | empty, empty
        else
          AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%R"));  // Title, Rating | empty, empty

        SetSortMethod(g_settings.m_viewStateVideoNavEpisodes.m_sortMethod);

        SetViewAsControl(g_settings.m_viewStateVideoNavEpisodes.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavEpisodes.m_sortOrder);
        break;
      }
    case NODE_TYPE_RECENTLY_ADDED_EPISODES:
      {
        AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS("%Z - %H. %T", "%R"));  // TvShow - Order. Title, Rating | empty, empty
        SetSortMethod(SORT_METHOD_NONE);

        SetViewAsControl(g_settings.m_viewStateVideoNavEpisodes.m_viewMode);
        SetSortOrder(SortOrderNone);

        break;
      }
    case NODE_TYPE_TITLE_MOVIES:
      {
        if (params.GetSetId() > -1) // Is this a listing within a set?
        {
          AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty

          if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
            AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE, 551, LABEL_MASKS("%T", "%R"));  // Title, Rating | empty, empty
          else
            AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE, 551, LABEL_MASKS("%T", "%R"));  // Title, Rating | empty, empty
        }
        else
        {
          if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
            AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE, 551, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
          else
            AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE, 551, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating

          AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T", "%Y", "%T", "%Y"));  // Title, Year | Title, Year
        }
        AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
        AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%T", "%O"));  // Title, MPAA | empty, empty
        AddSortMethod(SORT_METHOD_VIDEO_RUNTIME, 2050, LABEL_MASKS("%T", "%D"));  // Title, Duration | empty, empty
        AddSortMethod(SORT_METHOD_DATEADDED, 570, LABEL_MASKS("%T", "%a", "%T", "%a"));  // Title, DateAdded | Title, DateAdded

        if (g_settings.GetWatchMode(items.GetContent()) == VIDEO_SHOW_ALL)
          AddSortMethod(SORT_METHOD_PLAYCOUNT, 576, LABEL_MASKS("%T", "%V", "%T", "%V"));  // Title, Playcount | Title, Playcount

        if (params.GetSetId() > -1)
          SetSortMethod(SORT_METHOD_YEAR);
        else
          SetSortMethod(g_settings.m_viewStateVideoNavTitles.m_sortMethod);

        SetViewAsControl(g_settings.m_viewStateVideoNavTitles.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavTitles.m_sortOrder);
      }
      break;
      case NODE_TYPE_TITLE_MUSICVIDEOS:
      {
        if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
          AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 556, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty
        else
          AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty
        AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%T", "%O"));
        AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty
        if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
        {
          AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
          AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T", "%Y"));  // Album - Title, Year | empty, empty
        }
        else
        {
          AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
          AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T", "%Y"));  // Album - Title, Year | empty, empty
        }

        if (g_settings.GetWatchMode(items.GetContent()) == VIDEO_SHOW_ALL)
          AddSortMethod(SORT_METHOD_PLAYCOUNT, 576, LABEL_MASKS("%T", "%V"));  // Title, Playcount | empty, empty

        CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
        CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");
        AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty

        SetSortMethod(g_settings.m_viewStateVideoNavMusicVideos.m_sortMethod);

        SetViewAsControl(g_settings.m_viewStateVideoNavMusicVideos.m_viewMode);

        SetSortOrder(g_settings.m_viewStateVideoNavMusicVideos.m_sortOrder);
      }
      break;
    case NODE_TYPE_RECENTLY_ADDED_MOVIES:
      {
        AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS("%T", "%R"));  // Title, Rating | empty, empty
        SetSortMethod(SORT_METHOD_NONE);

        SetViewAsControl(g_settings.m_viewStateVideoNavTitles.m_viewMode);

        SetSortOrder(SortOrderNone);
      }
      break;
    case NODE_TYPE_RECENTLY_ADDED_MUSICVIDEOS:
      {
        AddSortMethod(SORT_METHOD_NONE, 552, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
        SetSortMethod(SORT_METHOD_NONE);

        SetViewAsControl(g_settings.m_viewStateVideoNavMusicVideos.m_viewMode);

        SetSortOrder(SortOrderNone);
      }
      break;
    default:
      break;
    }
  }
  else
  {
    if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
      AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty
    else
      AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty
    AddSortMethod(SORT_METHOD_SIZE, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));  // Label, Size | Label, Size
    AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));  // Label, Date | Label, Date
    AddSortMethod(SORT_METHOD_FILE, 561, LABEL_MASKS("%L", "%I", "%L", ""));  // Label, Size | Label, empty
    
    SetSortMethod(g_settings.m_viewStateVideoFiles.m_sortMethod);
    SetViewAsControl(g_settings.m_viewStateVideoFiles.m_viewMode);
    SetSortOrder(g_settings.m_viewStateVideoFiles.m_sortOrder);
  }
  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateWindowVideoNav::SaveViewState()
{
  if (m_items.IsVideoDb())
  {
    NODE_TYPE NodeType = CVideoDatabaseDirectory::GetDirectoryChildType(m_items.GetPath());
    CQueryParams params;
    CVideoDatabaseDirectory::GetQueryParams(m_items.GetPath(),params);
    switch (NodeType)
    {
    case NODE_TYPE_ACTOR:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavActors);
      break;
    case NODE_TYPE_YEAR:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavYears);
      break;
    case NODE_TYPE_GENRE:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavGenres);
      break;
    case NODE_TYPE_TITLE_MOVIES:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, params.GetSetId() > -1 ? NULL : &g_settings.m_viewStateVideoNavTitles);
      break;
    case NODE_TYPE_EPISODES:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavEpisodes);
      break;
    case NODE_TYPE_TITLE_TVSHOWS:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavTvShows);
      break;
    case NODE_TYPE_SEASONS:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavSeasons);
      break;
    case NODE_TYPE_TITLE_MUSICVIDEOS:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavMusicVideos);
      break;
    default:
      SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV);
      break;
    }
  }
  else
  {
    SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoFiles);
  }
}

VECSOURCES& CGUIViewStateWindowVideoNav::GetSources()
{
  //  Setup shares we want to have
  m_sources.clear();
  CFileItemList items;
  if (g_settings.m_bMyVideoNavFlatten)
    CDirectory::GetDirectory("library://video_flat/", items, "");
  else
    CDirectory::GetDirectory("library://video/", items, "");
  for (int i=0; i<items.Size(); ++i)
  {
    CFileItemPtr item=items[i];
    CMediaSource share;
    share.strName=item->GetLabel();
    share.strPath = item->GetPath();
    share.m_strThumbnailImage= item->GetIconImage();
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }
  return CGUIViewStateWindowVideo::GetSources();
}

bool CGUIViewStateWindowVideoNav::AutoPlayNextItem()
{
  CQueryParams params;
  CVideoDatabaseDirectory::GetQueryParams(m_items.GetPath(),params);
  if (params.GetContentType() == VIDEODB_CONTENT_MUSICVIDEOS || params.GetContentType() == 6) // recently added musicvideos
    return g_guiSettings.GetBool("musicplayer.autoplaynextitem");

  return g_guiSettings.GetBool("videoplayer.autoplaynextitem");
}

CGUIViewStateWindowVideoPlaylist::CGUIViewStateWindowVideoPlaylist(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  AddSortMethod(SORT_METHOD_NONE, 551, LABEL_MASKS("%L", "", "%L", ""));  // Label, empty | Label, empty
  SetSortMethod(SORT_METHOD_NONE);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SortOrderNone);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_PLAYLIST);
}

void CGUIViewStateWindowVideoPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_PLAYLIST);
}

bool CGUIViewStateWindowVideoPlaylist::HideExtensions()
{
  return true;
}

bool CGUIViewStateWindowVideoPlaylist::HideParentDirItems()
{
  return true;
}

VECSOURCES& CGUIViewStateWindowVideoPlaylist::GetSources()
{
  m_sources.clear();
  //  Playlist share
  CMediaSource share;
  share.strPath= "playlistvideo://";
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  // no plugins in playlist window
  return m_sources;
}


CGUIViewStateVideoMovies::CGUIViewStateVideoMovies(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
    AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE, 551, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
  else
    AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE, 551, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
  AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
  AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%T", "%O"));  // Title, MPAA | empty, empty
  AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T", "%Y", "%T", "%Y"));  // Title, Year | Title, Year

  if (items.IsSmartPlayList() || items.GetProperty("library.filter").asBoolean())
  {
    AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS("%T", "%R", "%T", "%R"));  // Title, Rating | Title, Rating
    SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);
  }
  else
    SetSortMethod(g_settings.m_viewStateVideoNavTitles.m_sortMethod);

  SetViewAsControl(g_settings.m_viewStateVideoNavTitles.m_viewMode);

  SetSortOrder(g_settings.m_viewStateVideoNavTitles.m_sortOrder);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoMovies::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavTitles);
}


CGUIViewStateVideoMusicVideos::CGUIViewStateVideoMusicVideos(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 556, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty
  else
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty
  AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%T", "%O"));
  AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T", "%Y"));  // Title, Year | empty, empty
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
  {
    AddSortMethod(SORT_METHOD_ARTIST_IGNORE_THE, 557, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
    AddSortMethod(SORT_METHOD_ALBUM_IGNORE_THE, 558, LABEL_MASKS("%B - %T", "%Y"));  // Album - Title, Year | empty, empty
  }
  else
  {
    AddSortMethod(SORT_METHOD_ARTIST, 557, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
    AddSortMethod(SORT_METHOD_ALBUM, 558, LABEL_MASKS("%B - %T", "%Y"));  // Album - Title, Year | empty, empty
  }

  AddSortMethod(SORT_METHOD_PLAYCOUNT, 576, LABEL_MASKS("%T", "%V"));  // Title, Playcount | empty, empty

  CStdString strTrackLeft=g_guiSettings.GetString("musicfiles.trackformat");
  CStdString strTrackRight=g_guiSettings.GetString("musicfiles.trackformatright");
  AddSortMethod(SORT_METHOD_TRACKNUM, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty

  if (items.IsSmartPlayList() || items.GetProperty("library.filter").asBoolean())
  {
    AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS("%A - %T", "%Y"));  // Artist - Title, Year | empty, empty
    SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);
  }
  else
    SetSortMethod(g_settings.m_viewStateVideoNavMusicVideos.m_sortMethod);

  SetViewAsControl(g_settings.m_viewStateVideoNavMusicVideos.m_viewMode);

  SetSortOrder(g_settings.m_viewStateVideoNavMusicVideos.m_sortOrder);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoMusicVideos::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavMusicVideos);
}

CGUIViewStateVideoTVShows::CGUIViewStateVideoTVShows(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
    AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE_IGNORE_THE, 551, LABEL_MASKS("%T", "%M", "%T", "%M"));  // Title, #Episodes | Title, #Episodes
  else
    AddSortMethod(SORT_METHOD_VIDEO_SORT_TITLE, 551, LABEL_MASKS("%T", "%M", "%T", "%M"));  // Title, #Episodes | Title, #Episodes
  AddSortMethod(SORT_METHOD_YEAR, 562, LABEL_MASKS("%T", "%Y", "%T", "%Y"));  // Title, Year | Title, Year

  if (items.IsSmartPlayList() || items.GetProperty("library.filter").asBoolean())
  {
    AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS("%T", "%M", "%T", "%M"));  // Title, #Episodes | Title, #Episodes
    SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);
  }
  else
    SetSortMethod(g_settings.m_viewStateVideoNavTvShows.m_sortMethod);

  SetViewAsControl(g_settings.m_viewStateVideoNavTvShows.m_viewMode);

  SetSortOrder(g_settings.m_viewStateVideoNavTvShows.m_sortOrder);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoTVShows::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavTvShows);
}


CGUIViewStateVideoEpisodes::CGUIViewStateVideoEpisodes(const CFileItemList& items) : CGUIViewStateWindowVideo(items)
{
  if (g_guiSettings.GetBool("filelists.ignorethewhensorting"))
    AddSortMethod(SORT_METHOD_LABEL_IGNORE_THE, 551, LABEL_MASKS("%Z - %H. %T","%R"));  // TvShow - Order. Title, Rating | empty, empty
  else
    AddSortMethod(SORT_METHOD_LABEL, 551, LABEL_MASKS("%Z - %H. %T", "%R"));  // TvShow - Order. Title, Rating | empty, empty
  if (0)//params.GetSeason() > -1)
  {
    AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%E. %T", "%R"));  // Episode. Title, Rating | empty, empty
    AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%E. %T", "%O"));  // Episode. Title, MPAA | empty, empty
    AddSortMethod(SORT_METHOD_EPISODE, 20359, LABEL_MASKS("%E. %T","%R"));  // Episode. Title, Rating | empty, empty
    AddSortMethod(SORT_METHOD_PRODUCTIONCODE, 20368, LABEL_MASKS("%E. %T","%P", "%E. %T","%P"));  // Episode. Title, Production Code | Episode. Title, Production Code
    AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%E. %T","%J","E. %T","%J"));  // Episode. Title, Date | Episode. Title, Date
  }
  else
  { // format here is tvshowtitle - season/episode number. episode title
    AddSortMethod(SORT_METHOD_VIDEO_RATING, 563, LABEL_MASKS("%Z - %H. %T", "%R"));  // TvShow - Order. Title, Rating | empty, empty
    AddSortMethod(SORT_METHOD_MPAA_RATING, 20074, LABEL_MASKS("%Z - %H. %T", "%O"));  // TvShow - Order. Title, MPAA | empty, empty
    AddSortMethod(SORT_METHOD_EPISODE, 20359, LABEL_MASKS("%Z - %H. %T","%R"));  // TvShow - Order. Title, Rating | empty, empty
    AddSortMethod(SORT_METHOD_PRODUCTIONCODE, 20368, LABEL_MASKS("%Z - %H. %T","%P"));  // TvShow - Order. Title, Production Code | empty, empty
    AddSortMethod(SORT_METHOD_DATE, 552, LABEL_MASKS("%Z - %H. %T","%J"));  // TvShow - Order. Title, Date | empty, empty
  }

  if (items.IsSmartPlayList() || items.GetProperty("library.filter").asBoolean())
  {
    AddSortMethod(SORT_METHOD_PLAYLIST_ORDER, 559, LABEL_MASKS("%Z - %H. %T", "%R"));  // TvShow - Order. Title, Rating | empty, empty
    SetSortMethod(SORT_METHOD_PLAYLIST_ORDER);
  }
  else
    SetSortMethod(g_settings.m_viewStateVideoNavEpisodes.m_sortMethod);

  SetViewAsControl(g_settings.m_viewStateVideoNavEpisodes.m_viewMode);

  SetSortOrder(g_settings.m_viewStateVideoNavEpisodes.m_sortOrder);

  LoadViewState(items.GetPath(), WINDOW_VIDEO_NAV);
}

void CGUIViewStateVideoEpisodes::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_VIDEO_NAV, &g_settings.m_viewStateVideoNavEpisodes);
}

