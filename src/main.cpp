#include "render.h"
#include "route_model.h"
#include "route_planner.h"
#include <fstream>
#include <io2d.h>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace std::experimental;

//-- Main heleper functions declarations----------------------
static std::optional<std::vector<std::byte>> ReadFile(const std::string &path);
float get_user_input(const std::string &coordinate);

int main(int argc, const char **argv) {
  std::string osm_data_file = "";
  if (argc > 1) {
    for (int i = 1; i < argc; ++i)
      if (std::string_view{argv[i]} == "-f" && ++i < argc)
        osm_data_file = argv[i];
  } else {
    std::cout << "To specify a map file use the following format: "
              << std::endl;
    std::cout << "Usage: [executable] [-f filename.osm]" << std::endl;
    osm_data_file = "../map.osm";
  }

  std::vector<std::byte> osm_data;

  if (osm_data.empty() && !osm_data_file.empty()) {
    std::cout << "Reading OpenStreetMap data from the following file: "
              << osm_data_file << std::endl;
    auto data = ReadFile(osm_data_file);
    if (!data)
      std::cout << "Failed to read." << std::endl;
    else
      osm_data = std::move(*data);
  }

  float start_x = get_user_input("start X");
  float start_y = get_user_input("start Y");
  float end_x = get_user_input("end X");
  float end_y = get_user_input("end Y");

  // Build Model.
  RouteModel model{osm_data};

  // Create RoutePlanner object and perform A* search.
  RoutePlanner route_planner{model, start_x, start_y, end_x, end_y};
  route_planner.AStarSearch();

  std::cout << "Distance: " << route_planner.GetDistance() << " meters. \n";

  // Render results of search.
  Render render{model};

  auto display = io2d::output_surface{400,
                                      400,
                                      io2d::format::argb32,
                                      io2d::scaling::none,
                                      io2d::refresh_style::fixed,
                                      30};
  display.size_change_callback([](io2d::output_surface &surface) {
    surface.dimensions(surface.display_dimensions());
  });
  display.draw_callback(
      [&](io2d::output_surface &surface) { render.Display(surface); });
  display.begin_show();
}

//##################################################################
// Main helper functions definitions:

// gets single coordinate from the user and validates
// to fit in the 0 - 100 range
// the prompt for input is repeated until single coordinate is ok
float get_user_input(const std::string &coordinate) {
  float val{};
  do {
    std::cout << "Please enter the " + coordinate +
                     " coordinate of the route (0 to 100): ";
    std::string line{};
    std::getline(std::cin, line);
    std::istringstream lstream(line);
    if (!(lstream >> val)) {
      // if erroneus read of float - set val to value enforcing another try
      val = -1; // -1 won't pass the while test
    }
  } while (val < 0.0f || val > 100.0f);

  return val;
}

static std::optional<std::vector<std::byte>> ReadFile(const std::string &path) {
  std::ifstream is{path, std::ios::binary | std::ios::ate};
  if (!is)
    return std::nullopt;

  auto size = is.tellg();
  std::vector<std::byte> contents(size);

  is.seekg(0);
  is.read((char *)contents.data(), size);

  if (contents.empty())
    return std::nullopt;
  return std::move(contents);
}