# HTTP Router

Router library based on Boost.Beast

```cmake
target_link_libraries(server PRIVATE http-router http-router-filters)
```

```c++
#include <filesystem>
#include <fmt/format.h>
#include <http-router/filters/static_files.hh>
#include <http-router/server.hh>
#include <http-router/version.hh>
#include <my_app/dirinfo.hh>
#include <my_app/overlay_dirs.hh>
#include <my_app/cmdline.hh>

service::service(cmdline::args const& args) {
    using namespace http_router::filters;
    using info = dirinfo::info;
    using tcp = boost::asio::ip::tcp;

    auto cfg = cmdline::parse(args);

    m_app = router::cfg {}
#ifndef NDEBUG
        .use(static_files<root_directory>::make(
            info::source_dir / "data/www/static"))
#endif
        .use(static_files<named_root_directories>::make(
            make_overlay_dirs("www", cfg.verbose),
            fmt::format("/{}/www", dirinfo::shared_dir)))
        .append("/api/v1", rest::v1::api{})
        .get("/app", on_app)
        .compiled();

    if (cfg.verbose) app.print();

    using http_router::version;
    auto server = fmt::format(
        "{}/{}.{}", cfg.progname, version::major, version::minor);
    server::listen(m_ioc, tcp::endpoint{tcp::v4(), cfg.port},
                   server, &m_app);
}
```