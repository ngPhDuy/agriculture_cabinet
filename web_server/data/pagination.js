document.addEventListener('DOMContentLoaded', function() {
    const rowsPerPage = 6;
    const table = document.getElementById('table').getElementsByTagName('tbody')[0];
    const rows = table.getElementsByTagName('tr');
    const pagination = document.getElementById('pagination');
    const pageNumbers = pagination.getElementsByClassName('page-num');
    const prev = pagination.getElementsByClassName('prev')[0];
    const next = pagination.getElementsByClassName('next')[0];
    let currentPage = 1;

    console.log(rows);
    console.log(rows.length);

    function displayRows(startIndex, endIndex) {
        for (let i = 0; i < rows.length; i++) {
            rows[i].style.display = (i >= startIndex && i < endIndex) ? '' : 'none';
        }
    }

    function updatePagination() {
        const totalPages = Math.ceil(rows.length / rowsPerPage);

        if (totalPages === 1) {
            pagination.style.display = 'none';
            return;
        }
        
        while (pagination.firstChild) {
            pagination.removeChild(pagination.firstChild);
        }
        
        pagination.appendChild(prev);

        for (let i = 1; i <= totalPages; i++) {
            let page = document.createElement('a');
            page.href = "#";
            page.textContent = i;
            page.classList.add('page-num');
            if (i === currentPage) {
                page.classList.add('active');
            }
            page.addEventListener('click', function(e) {
                e.preventDefault();
                currentPage = i;
                displayRows((currentPage - 1) * rowsPerPage, currentPage * rowsPerPage);
                updatePagination();
            });
            pagination.appendChild(page);
        }
        
        pagination.appendChild(next);
        prev.style.display = currentPage === 1 ? 'none' : '';
        next.style.display = currentPage === totalPages ? 'none' : '';
    }

    prev.addEventListener('click', function(e) {
        e.preventDefault();
        if (currentPage > 1) {
            currentPage--;
            displayRows((currentPage - 1) * rowsPerPage, currentPage * rowsPerPage);
            updatePagination();
        }
    });

    next.addEventListener('click', function(e) {
        e.preventDefault();
        if (currentPage < Math.ceil(rows.length / rowsPerPage)) {
            currentPage++;
            displayRows((currentPage - 1) * rowsPerPage, currentPage * rowsPerPage);
            updatePagination();
        }
    });

    displayRows(0, rowsPerPage);
    updatePagination();
});
