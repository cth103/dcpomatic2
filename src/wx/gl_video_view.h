/*
    Copyright (C) 2019 Carl Hetherington <cth@carlh.net>

    This file is part of DCP-o-matic.

    DCP-o-matic is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    DCP-o-matic is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with DCP-o-matic.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "video_view.h"
#include "lib/signaller.h"
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <dcp/util.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#undef None
#undef Success

class GLVideoView : public VideoView
{
public:
	GLVideoView (FilmViewer* viewer, wxWindow* parent);
	~GLVideoView ();

	void set_image (boost::shared_ptr<const Image> image);
	wxWindow* get () const {
		return _canvas;
	}
	void update ();
	void start ();
	void stop ();

	bool display_next_frame (bool);

	bool vsync_enabled () const {
		return _vsync_enabled;
	}

private:
	void paint ();
	void draw ();
	void thread ();
	void request_one_shot ();
	void create ();
	void check_for_butler_errors ();

	wxGLCanvas* _canvas;
	wxGLContext* _context;

	GLuint _id;
	boost::optional<dcp::Size> _size;
	bool _vsync_enabled;
	boost::thread* _thread;

	boost::mutex _playing_mutex;
	boost::condition _playing_condition;
	bool _playing;
	bool _one_shot;

	boost::shared_ptr<wxTimer> _timer;
};
