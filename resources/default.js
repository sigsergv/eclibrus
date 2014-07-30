function exportAllBooksFromPage()
{
    window.document.location.href = "http://lenta.ru";
}

function expandBlock(block_id, link_on_id, link_off_id)
{
    var link_on = document.getElementById(link_on_id);
    var link_off = document.getElementById(link_off_id);
    var block = document.getElementById(block_id);

    block.style.display = 'block';
    link_on.style.display = 'none';
    link_off.style.display = 'inline';
}

function collapseBlock(block_id, link_on_id, link_off_id)
{
    var link_on = document.getElementById(link_on_id);
    var link_off = document.getElementById(link_off_id);
    var block = document.getElementById(block_id);

    block.style.display = 'none';
    link_on.style.display = 'inline';
    link_off.style.display = 'none';
}